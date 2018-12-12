#include "../../includes/worst_case_timer.h"

using namespace std;
using namespace timers_utils;

WorstCaseTimer::WorstCaseTimer(uint_fast32_t worst_timer_delay_us) : error_handler_("WorstCaseTimer") {
	worst_timer_delay_ns_ = US_TO_NS(worst_timer_delay_us);
	frequency_ = TIMER_FREQUNCY_ERROR;
	timer_started_ = false;
}

bool WorstCaseTimer::initialize() {
	frequency_ = TIMER_FREQUNCY_ERROR;
	timer_started_ = false;
	return true;
}

bool WorstCaseTimer::set_frequency(uint_fast16_t frequency) {
	//check the frequency is under limit
	if (frequency > TIMER_MAX_FREQUENCY) {
		sprintf (buffer_, "The frequency provided is : %d, the max frequency supported is %d", frequency, TIMER_MAX_FREQUENCY);
		error_handler_.set_error(buffer_);
		return false;
	}

	frequency_ = frequency;
	original_sleep_time_ps_ = SEC_TO_PS(1) / frequency;
	return true;
}

uint_fast16_t WorstCaseTimer::get_frequency() {
	return frequency_;
}

bool WorstCaseTimer::start_timer() {

	//check if the timer already started
	if(timer_started_) {
		error_handler_.set_error("You can't start the timer while it is already running");
		return false;
	}

	//set the starting timer and the flag
	clock_gettime(CLOCK_MONOTONIC, &start_time_);
	timer_started_ = true;
	
	//set the time for the next tick
	sleep_counter_ = 1;
	sleep_time_ns_ = PS_TO_NS(original_sleep_time_ps_ * sleep_counter_) + 1;	// +1 is for round-off errors

	//return true as a flag that the timer started
	return true;
}

void WorstCaseTimer::stop_timer() {
	timer_started_ = false;
}

bool WorstCaseTimer::sleep_to_next_tick() {

	//check if the tick already happened before calling this function
	timespec current_time;
	clock_gettime(CLOCK_MONOTONIC, &current_time);

	if(TIMESPEC_DIFF_NS(start_time_, current_time) > sleep_time_ns_) {
		error_handler_.set_error("The tick already ticked before calling sleep_to_next_tick.");
		return false;
	}

	//else, check if the needed sleep period less than the worst_timer_delay_
	if(sleep_time_ns_ - TIMESPEC_DIFF_NS(start_time_, current_time) < worst_timer_delay_ns_) {
		//then busy wait for the period
		while(TIMESPEC_DIFF_NS(start_time_, current_time) < sleep_time_ns_) {
			clock_gettime(CLOCK_MONOTONIC, &current_time);
		}
 	} else {
 		//block the process for the needed sleep time - worst_timer_delay_
		nanoseconds_sleep(sleep_time_ns_ - TIMESPEC_DIFF_NS(start_time_, current_time) - worst_timer_delay_ns_);
 		//then busy wait for the rest of the time
		while(TIMESPEC_DIFF_NS(start_time_, current_time) < sleep_time_ns_) {
			clock_gettime(CLOCK_MONOTONIC, &current_time);
		}
 	}

	/**
	 * prevent overflow:
	 * if this code runs for really long time the (original_sleep_time_ps_ * sleep_counter_) term
	 * will overflow since original_sleep_time_ps_ can be at max 10^12 and sleep_counter_ increases.
	 * so we will reset the timer when sleep_counter_ reaches 2^23.
	 * why do not do this after each iteration? - since sleep_time_ns_ is bigger than the needed
	 * value by 1 nanosecond because round-off error.
	 * so this increases the error by 1ns each (2^23 / frequency) seconds.
	 */
	if(sleep_counter_ == (uint_fast32_t (1 << 23))) {
		//reset the start_time_ to the current timer
		ADD_NS_TO_TIMESPEC(start_time_, sleep_time_ns_);
		//reset the counter
		sleep_counter_ = 0;
	}

	//set the time for the next tick
	sleep_counter_++;
	sleep_time_ns_ = PS_TO_NS(original_sleep_time_ps_ * sleep_counter_) + 1;	// +1 is for round-off errors

	return true;
}

string WorstCaseTimer::get_error() {
	string error = error_handler_.get_error();
	error_handler_.clear_error();
	return error;
}

bool WorstCaseTimer::is_error() {
	return error_handler_.is_error();
}

WorstCaseTimer::~WorstCaseTimer() {
	
}