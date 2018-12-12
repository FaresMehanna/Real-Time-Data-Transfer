#include "../../includes/timers_utils.h"

namespace timers_utils{

	/**
	 * functions : Linux based sleep functions
	 * -------------------------------
	 */

	void nanoseconds_sleep(uint_fast64_t sleep_time_ns) {

		timespec start_time, current_time;
		uint_fast64_t slept_time_ns;
		struct timespec ns_sleep_time;

		clock_gettime(CLOCK_MONOTONIC, &start_time);
		clock_gettime(CLOCK_MONOTONIC, &current_time);


		while((slept_time_ns = TIMESPEC_DIFF_NS(start_time, current_time)) < sleep_time_ns) {

			ns_sleep_time.tv_sec = NS_TO_SEC(sleep_time_ns - slept_time_ns);
			ns_sleep_time.tv_nsec = long((sleep_time_ns - slept_time_ns) % SEC_TO_NS(1));
			nanosleep(&ns_sleep_time , NULL);

			clock_gettime(CLOCK_MONOTONIC, &current_time);
		}

	}

	void microseconds_sleep(uint_fast32_t sleep_time_us) {
		nanoseconds_sleep(US_TO_NS(sleep_time_us));
	}

	void milliseconds_sleep(uint_fast32_t sleep_time_ms) {
		nanoseconds_sleep(MS_TO_NS(sleep_time_ms));
	}

	void seconds_sleep(uint_fast16_t sleep_time_s) {
		nanoseconds_sleep(SEC_TO_NS(sleep_time_s));
	}

}