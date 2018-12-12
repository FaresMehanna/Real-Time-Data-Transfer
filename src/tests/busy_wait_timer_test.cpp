#include <iostream>

#include "../../includes/senders.h"
#include "../../includes/systems.h"
#include "../../includes/timers.h"

using namespace std;
using namespace timers_utils;

int main(void) {

	BusyWaitTimer timer;

	if(!timer.initialize()) {
		cout << "Failed to initialize the busy_wait_timer." << endl;
		cout << timer.get_error() << endl;
		return 1;
	}

	if(!timer.set_frequency(100)) {
		cout << "Failed to set the frequency." << endl;
		cout << timer.get_error() << endl;
		return 2;
	}

	if(!timer.start_timer()) {
		cout << "Failed to start the busy_wait_timer." << endl;
		cout << timer.get_error() << endl;
		return 3;
	}

	timespec start_time;
	clock_gettime(CLOCK_MONOTONIC, &start_time);

	while(timer.sleep_to_next_tick()) {

		timespec end_time;
		clock_gettime(CLOCK_MONOTONIC, &end_time);

		cout << NS_TO_US(TIMESPEC_DIFF_NS(start_time, end_time)) << endl;

		start_time = end_time;
	}
	cout << "Failed in calling sleep_to_next_tick." << endl;
	cout << timer.get_error() << endl;
	return 4;
}