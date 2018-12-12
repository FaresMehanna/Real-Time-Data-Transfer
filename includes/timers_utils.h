#ifndef SRC_TIMERS_TIMERS_UTILS_
#define SRC_TIMERS_TIMERS_UTILS_

#include <stdint.h>
#include <time.h>
#include <unistd.h>

namespace timers_utils{

	/**
	 * Macros : Time conversions
	 * -------------------------------
	 * convert between different representations of time
	 * PS  => Picosecond
	 * NS  => Nanosecond
	 * US  => Microsecond 
	 * MS  => Millisecond
	 * SEC => Second
	 */
	#define SEC_TO_MS(S) ((S) * uint_fast64_t (1000))
	#define SEC_TO_US(S) (SEC_TO_MS(S) * uint_fast64_t (1000))
	#define SEC_TO_NS(S) (SEC_TO_US(S) * uint_fast64_t (1000))
	#define SEC_TO_PS(S) (SEC_TO_NS(S) * uint_fast64_t (1000))

	#define MS_TO_SEC(S) ((S) / uint_fast64_t (1000))
	#define MS_TO_US(S)  ((S) * uint_fast64_t (1000))
	#define MS_TO_NS(S)  (MS_TO_US(S) * uint_fast64_t (1000))
	#define MS_TO_PS(S)  (MS_TO_NS(S) * uint_fast64_t (1000))

	#define US_TO_MS(S)  ((S) / uint_fast64_t (1000))
	#define US_TO_SEC(S) (US_TO_MS(S) / uint_fast64_t (1000))
	#define US_TO_NS(S)  ((S) * uint_fast64_t (1000))
	#define US_TO_PS(S)  (US_TO_NS(S) * uint_fast64_t (1000))

	#define NS_TO_US(S)  ((S) / uint_fast64_t (1000))
	#define NS_TO_MS(S)  (NS_TO_US(S) / uint_fast64_t (1000))
	#define NS_TO_SEC(S) (NS_TO_MS(S) / uint_fast64_t (1000))
	#define NS_TO_PS(S)  ((S) * uint_fast64_t (1000))

	#define PS_TO_NS(S)  ((S) / uint_fast64_t (1000))
	#define PS_TO_US(S)  (PS_TO_NS(S) / uint_fast64_t (1000))
	#define PS_TO_MS(S)  (PS_TO_US(S) / uint_fast64_t (1000))
	#define PS_TO_SEC(S) (PS_TO_MS(S) / uint_fast64_t (1000))



	/**
	 * Macro : TIMESPEC_DIFF_NS
	 * -------------------------------
	 * take two timespec parameters.
	 * Return in nanoseconds as uint_fast64_t the difference between
	 * t1 and t2 where t1 captured before t2. 
	 */
	#define TIMESPEC_DIFF_NS(t1, t2) \
			(uint_fast64_t((SEC_TO_NS(t2.tv_sec-t1.tv_sec)) + (t2.tv_nsec-t1.tv_nsec)))

	#define ADD_NS_TO_TIMESPEC(timespec, ns) \
			timespec.tv_sec  += NS_TO_SEC(timespec.tv_nsec + (ns)); \
			timespec.tv_nsec = ((timespec.tv_nsec + (ns)) % SEC_TO_NS(1))

	/**
	 * functions : Linux based sleep functions
	 * -------------------------------
	 */

	void nanoseconds_sleep(uint_fast64_t sleep_time_ns);
	void microseconds_sleep(uint_fast32_t sleep_time_us);
	void milliseconds_sleep(uint_fast32_t sleep_time_ms);
	void seconds_sleep(uint_fast16_t sleep_time_s);


}

#endif