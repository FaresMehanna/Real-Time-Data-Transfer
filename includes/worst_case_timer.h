#ifndef SRC_TIMERS_WORST_CASE_TIMER_H
#define SRC_TIMERS_WORST_CASE_TIMER_H

#include <string>
#include <stdint.h>

#include "timer.h"
#include "timers_utils.h"
#include "error.h"

class WorstCaseTimer : public Timer{

private:

	//the time when start_timer() called
	timespec start_time_;

	//frequency provided by the user
	uint_fast16_t frequency_;

	//is the sleep time in picoseconds (10^12/frequency)
	uint_fast64_t original_sleep_time_ps_;

	//is the target sleep time between start_time_ and current time.
	uint_fast64_t sleep_time_ns_;

	//is the timer currently running
	bool timer_started_;

	//counter of the timer
	uint_fast32_t sleep_counter_;

	//counter of the timer
	uint_fast64_t worst_timer_delay_ns_;

	//buffer for error logging
	char buffer_ [256];

	//error handler class
	Error error_handler_;

public:

	/**
	 * Method : Constructor
	 * -------------------------------
	 * The constructor accept one argument which the the worst time the OS can delay 
	 * the process. the way it's work is that the timer will block the process for the sleeping
	 * time - worst_timer_delay_us. then will do busy wait for the rest of the time.
	 */
	WorstCaseTimer(uint_fast32_t worst_timer_delay_us);

	bool initialize() override;

	bool set_frequency(uint_fast16_t _frequency) override;

	uint_fast16_t get_frequency() override;

	bool start_timer() override;

	void stop_timer() override;

	bool sleep_to_next_tick() override;

	std::string get_error() override;

	bool is_error() override;

	~WorstCaseTimer();

};

#endif