#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sched.h>

#include "../../includes/senders.h"
#include "../../includes/systems.h"
#include "../../includes/timers.h"

using namespace std;

void* data_to_be_sent;
int message_size;

DataPacketsList my_fn(RealTimeInfo* inf) {

	if(inf->is_skipped_data()) {
		cout << "Skipped Frame Detected." << endl;
	}

	DataPacket packet_1;
	packet_1.data_ptr = data_to_be_sent;
	packet_1.data_size = message_size;

	DataPacketsList list(1);
	(list.packets)[0] = packet_1;

	return list;
}

int main(int argc, char** argv) {

	if(argc != 3) {
		printf("Real time system - always on - test\n");
		printf("\n");
		printf("Usage:\n");
		printf("%s port_number single_message_size_in_bytes", argv[0]);
		exit(0);
	}

	int port_number = atoi(argv[1]);
	int frequency = 30;
	message_size = atoi(argv[2]);
	cout << message_size << endl;
	data_to_be_sent = malloc(message_size);

	TCPSender sender(port_number);
	FreeWaitTimer timer;

	RealTimeSystem system;

	if(!system.set_timer(&timer)) {
		cout << "Failed to set the timer object." << endl;
		cout << system.get_error() << endl;
		return 1;
	}

	if(!system.set_sender(&sender)) {
		cout << "Failed to set the sender object." << endl;
		cout << system.get_error() << endl;
		return 2;
	}

	if(!system.set_user_data_fn(&my_fn)) {
		cout << "Failed to set the user function." << endl;
		cout << system.get_error() << endl;
		return 3;
	}

	if(!system.set_frequency(frequency)) {
		cout << "Failed to set the frequency." << endl;
		cout << system.get_error() << endl;
		return 4;
	}

	system.set_ms_tolerance(1000/frequency);
	system.skip_mode(true);

	while(1) {
		if(!system.initialize()) {
			cout << "Failed to initialize the system." << endl;
			cout << system.get_error() << endl;
			return 5;
		}
		
		system.run();
		cout << "Ended the current client." << endl;
	}

	return 6;
}

//g++ real_time_system_always_on_test.cpp ../senders/tcp_sender.cpp  ../systems/real_time_system.cpp ../timers/free_wait_timer.cpp ../timers/timers_utils.cpp ../utils/error.cpp  ../utils/mutex_raii.cpp -pthread -std=c++11 -o rtsaot
