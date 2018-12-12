#include "../../includes/busy_wait_timer.h"

using namespace std;
using namespace timers_utils;

BusyWaitTimer::BusyWaitTimer() : error_handler_("BusyWaitTimer") {
	frequency_ = TIMER_FREQUNCY_ERROR;
	timer_started_ = false;
}

bool BusyWaitTimer::initialize() {
	frequency_ = TIMER_FREQUNCY_ERROR;
	timer_started_ = false;
	return true;
}

bool BusyWaitTimer::set_frequency(uint_fast16_t frequency) {
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

uint_fast16_t BusyWaitTimer::get_frequency() {
	return frequency_;
}

bool BusyWaitTimer::start_timer() {

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

void BusyWaitTimer::stop_timer() {
	timer_started_ = false;
}

bool BusyWaitTimer::sleep_to_next_tick() {

	//check if the tick already happened before calling this function
	timespec current_time;
	clock_gettime(CLOCK_MONOTONIC, &current_time);
	if(TIMESPEC_DIFF_NS(start_time_, current_time) > sleep_time_ns_) {
		error_handler_.set_error("The tick already ticked before calling sleep_to_next_tick.");
		return false;
	}

	//else, let's busy wait for the sleep_time period
	while(TIMESPEC_DIFF_NS(start_time_, current_time) < sleep_time_ns_) {
		clock_gettime(CLOCK_MONOTONIC, &current_time);
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

string BusyWaitTimer::get_error() {
	string error = error_handler_.get_error();
	error_handler_.clear_error();
	return error;
}

bool BusyWaitTimer::is_error() {
	return error_handler_.is_error();
}

BusyWaitTimer::~BusyWaitTimer() {
	
}