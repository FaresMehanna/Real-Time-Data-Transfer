#ifndef SRC_TIMERS_TIMER_H_
#define SRC_TIMERS_TIMER_H_

#include <string>
#include <stdint.h>

#include "flags.h"

#define TIMER_FREQUNCY_ERROR (uint_fast16_t((uint_fast32_t (1 << 16)) - 1))

/**
 * Pure Abstract Class : Timer
 * -------------------------------
 * This abstract class MUST be inherited by each timer class.
 * Each derived class MUST implement ALL the methods defined in this
 * class.
 */
class Timer{

public:

	/**
	 * Method : initialize
	 * -------------------------------
	 * The method should try to initialize the timer.
	 * This will be the first method to be called from the system after 
	 * constructing the object.
	 * @return true if the timer correctly initialized, false in case of any
	 * error during the initialization.
	 * in case of false the method MUST report the error.
	 */
	virtual bool initialize() = 0;

	/**
	 * Method : set_frequency
	 * -------------------------------
	 * The method should change the internal frequency of the timer if
	 * possible, the method should respect TIMER_MAX_FREQUENCY limit.
	 * @param frequency is the frequency to set the timer
	 * @return true if the call before the timer starts and within the limit,
	 * and false otherwise. in case of false the method MUST report the error.
	 */
	virtual bool set_frequency(uint_fast16_t frequency) = 0;

	/**
	 * Method : get_frequency
	 * -------------------------------
	 * The method return the current frequency of the timer.
	 * @return number of frequency if it was set before, if not then it MUST
	 * return TIMER_FREQUNCY_ERROR
	 */
	virtual uint_fast16_t get_frequency() = 0;

	/**
	 * Method : start_timer
	 * -------------------------------
	 * The method start the timer in the exact time of calling it.
	 * @return true if the frequency was set and the timer started successfully,
	 * false otherwise, in case of false the method MUST report the error.
	 */
	virtual bool start_timer() = 0;

	/**
	 * Method : stop_timer
	 * -------------------------------
	 * The method should stop the timer if it was working. the method MUST NOT
	 * clear the frequency and the timer MUST be able to start again if needed.
	 */
	virtual void stop_timer() = 0;

	/**
	 * Method : sleep_to_next_tick
	 * -------------------------------
	 * The method should sleep until at least the next tick occur.
	 * @return true if the frequency was set and the timer started and the call
	 * occurred before the expected tick,false otherwise, in case of false the
	 * method MUST report the error.
	 */
	virtual bool sleep_to_next_tick() = 0;

	/**
	 * Method : get_error
	 * -------------------------------
	 * @return the error string if an error occurs and the error MUST be removed
	 * after this call, if no error occurs, return empty string ("").
	 */
	virtual std::string get_error() = 0;

	/**
	 * Method : is_error
	 * -------------------------------
	 * Check if an error existed.
	 * @return true if an error occurs, false otherwise.
	 */
	virtual bool is_error() = 0;

	virtual ~Timer() {};

};

#endif