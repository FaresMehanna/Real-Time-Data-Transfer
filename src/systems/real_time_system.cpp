#include "../../includes/real_time_system.h"

using namespace std;
using namespace timers_utils;

RealTimeSystem::RealTimeSystem() : error_handler_("RealTimeSystem") {
	timer_ = NULL;
	sender_ = NULL;
	user_app_func_ = NULL;
	allow_skip_mode_ = false;
	initialized_ = false;
	ms_tolerance_ = false;
	frequency_ = 0;
}

bool RealTimeSystem::set_timer(Timer* timer) {
	initialized_ = false;
	if(timer == NULL) {
		error_handler_.set_error("Null Timer provided");
		return false;
	}
	timer_ = timer;
	return true;
}

bool RealTimeSystem::set_sender(Sender* sender) {
	initialized_ = false;
	if(sender == NULL) {
		error_handler_.set_error("Null Sender provided");
		return false;
	}
	sender_ = sender;
	return true;
}

bool RealTimeSystem::set_user_data_fn(DataPacketsList (*user_app_func)(RealTimeInfo*)) {
	initialized_ = false;
	if(user_app_func == NULL) {
		error_handler_.set_error("Null Function provided");
		return false;
	}
	user_app_func_ = user_app_func;
	return true;
}

bool RealTimeSystem::set_frequency(uint_fast16_t frequency) {
	initialized_ = false;
	frequency_ = frequency;
	return true;
}

void RealTimeSystem::set_ms_tolerance(uint_fast16_t ms_tolerance) {
	initialized_ = false;
	//set tolerance
	ms_tolerance_ = ms_tolerance;
}

void RealTimeSystem::skip_mode(bool allow_skip_mode) {
	initialized_ = false;
	//set skip mode
	allow_skip_mode_ = allow_skip_mode;
}

bool RealTimeSystem::initialize() {

	//checks for not given data
	if(user_app_func_ == NULL) {
		error_handler_.set_error("Null user Function provided");
		return false;
	}
	if(timer_ == NULL) {
		error_handler_.set_error("Null Timer provided");
		return false;
	}
	if(sender_ == NULL) {
		error_handler_.set_error("Null Sender provided");
		return false;
	}
	if(frequency_ == 0) {
		error_handler_.set_error("Not provided frequency");
		return false;
	}

	//try to convert the process to real time one.
	if(!convert_rt_thread()) {
		return false;
	}
	//try to initialize the objects
	if(!timer_->initialize()) {
		error_handler_.set_error(timer_->get_error());
		return false;
	}
	//try to set the timer frequency
	if(!timer_->set_frequency(frequency_)) {
		error_handler_.set_error(timer_->get_error());
		return false;
	}
	//try to initialize the sender object
	if(!sender_->initialize()) {
		error_handler_.set_error(sender_->get_error());
		return false;
	}

	//adjust tolerance
	ms_tolerance_ = min(ms_tolerance_, (1000/frequency_) - 5);
	//set everything is good
	initialized_ = true;
	return true;
}

bool RealTimeSystem::send_before_run(DataPacketsList list_of_data, uint_fast32_t time_limit_ms) {
	
	//check initialization
	if(!initialized_) {
		error_handler_.set_error("System not initialized yet");
		return false;
	}

	//provide the data to the sender
	sender_->send(&(list_of_data));

	//log(n) sleeping
	while(time_limit_ms) {

		//calculate the sleep time
		uint_fast32_t sleeping_time = time_limit_ms/2 + 1;
		time_limit_ms -= sleeping_time;

		//sleep for the given time
		milliseconds_sleep(sleeping_time);

		//check the sender
		if(sender_->is_send_done()) {
			break;			
		}

	}

	//if sending done successfully
	if(sender_->is_send_done()) {
		return true;
	}

	//if not then shutdown the sender
	sender_->end_sender();
	error_handler_.set_error("Sender couldn't send the data in the given time limit");
	initialized_ = false;
	return false;
}


bool RealTimeSystem::run() {
	
	//check initialization
	if(!initialized_) {
		error_handler_.set_error("System not initialized yet.");
		return false;
	}
	//set the initialization flag to false again.
	initialized_ = false;

	//start the timer
	if(!timer_->start_timer()) {
		error_handler_.set_error(timer_->get_error());
		return false;
	}

	//all went good
	DataPacketsList user_packets_list(0);
	uint_fast32_t sequence_number = -1;
	uint_fast32_t skipped_count = 0;

	while(1) {
		//increase the sequence number
		sequence_number++;

		//check skip counter
		//this check here to detect if the client disconnected.
		if(skipped_count == frequency_*3) {
			error_handler_.set_error("The client disconnected.");
			sender_->end_sender();
			return false;
		}

		//if the sender not done yet, then let it consume as much as
		//wanted from the extra time.
		int used_tolerated_time = 0;
		for(; used_tolerated_time < ms_tolerance_*2; used_tolerated_time++) {
			if(sender_->is_send_done()) {
				break;			
			}
			microseconds_sleep(500);
		}

		//if after the extra time is not done yet, then check the skip_mode and act.
		if(!sender_->is_send_done() && !allow_skip_mode_) {
			error_handler_.set_error("Failed to send the data in the required time");
			sender_->end_sender();
			return false;
		}

		//if after the extra time is not done yet, then check the skip_mode and act.
		if(!sender_->is_send_done() && allow_skip_mode_) {
			//construct the info class
			RealTimeInfo user_info(sequence_number, true, used_tolerated_time/2, false);
			//call the user function then ignore the packets
			user_packets_list = user_app_func_(&user_info);
			//check if the user wants to end the system
			if(user_info.is_system_stopped()) {
				sender_->end_sender();
				return true;
			}
			//increment skip counter
			skipped_count++;
			//try to sleep
			if(!timer_->sleep_to_next_tick()) {
				error_handler_.set_error("Failed to return from the sender object in the needed time - this bug related to the sender object not to the size of the payload");
				sender_->end_sender();
				return false;
			}
			continue;
		}
		//empty skip counter
		skipped_count = 0;
		//construct the info class
		RealTimeInfo user_info(sequence_number, false, used_tolerated_time, false);
		//call the user function to get the packets
		user_packets_list = user_app_func_(&user_info);
		//check if the user wants to end the system
		if(user_info.is_system_stopped()) {
			sender_->end_sender();
			return true;
		}
		//then send the user data
		if(!sender_->send(&(user_packets_list))) {
			error_handler_.set_error(sender_->get_error());
			sender_->end_sender();
			return false;
		}
		//and sleep until the next timer tick
		if(!timer_->sleep_to_next_tick()) {
			error_handler_.set_error("Failed to return from the sender object in the needed time - this bug related to the sender object not to the size of the payload");
			sender_->end_sender();
			return false;
		}
	}

}

std::string RealTimeSystem::get_error() {
	std::string error = error_handler_.get_error();
	error_handler_.clear_error();
	return error;
}

bool RealTimeSystem::is_error() {
	return error_handler_.is_error();
}

bool RealTimeSystem::convert_rt_thread() {
        // Lock memory
        if(mlockall(MCL_CURRENT|MCL_FUTURE) == -1) {
		error_handler_.set_error("mlockall failed");
		return false;
        }
	//set scheduler to FIFO and with priority 99
	struct sched_param param;
	param.sched_priority = 99;
	if (sched_setscheduler(0, SCHED_FIFO, &param) != 0) {
		//report error
		error_handler_.set_error("Can't set the process scheduler to FIFO / set priority to 99");
		return false;
	}
	//set process affinity to core 0
	cpu_set_t cpuset;
	CPU_ZERO(&cpuset);
	CPU_SET(CPU_CORE_AFFINITY, &cpuset);
	if(sched_setaffinity(0, sizeof(cpu_set_t), &cpuset) != 0) {
		//report error
		error_handler_.set_error("Can't stick the process to CPU0");
		return false;
	}
	return true;
}

RealTimeSystem::~RealTimeSystem() {

}


/**
 * Class : RealTimeInfo
 * --------------------------------------------------------------
 */	
RealTimeInfo::RealTimeInfo(uint_fast32_t sequence_number, bool skip_next_data, uint_fast16_t delayed_ms, bool system_stopped) {
	sequence_number_ = sequence_number;
	skip_next_data_ = skip_next_data;
	delayed_ms_ = delayed_ms;
	system_stopped_ = system_stopped;
}

uint_fast32_t RealTimeInfo::get_sequence_number() {
	return sequence_number_;
}

uint_fast32_t RealTimeInfo::get_delayed_time_ms() {
	return delayed_ms_;
}

bool RealTimeInfo::is_skipped_data() {
	return skip_next_data_;
}

void RealTimeInfo::stop_system() {
	system_stopped_ = true;
}

bool RealTimeInfo::is_system_stopped() {
	return system_stopped_;
}
