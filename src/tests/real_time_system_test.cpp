#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sched.h>
#include <stdlib.h>

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

	if(inf->get_delayed_time_ms()) {
		cout << "Tolerance Used In Last Packet is " << inf->get_delayed_time_ms() << "." << endl;
	}

	DataPacket packet_1;
	packet_1.data_ptr = data_to_be_sent;
	packet_1.data_size = message_size;

	DataPacketsList list(1);
	(list.packets)[0] = packet_1;

	return list;
}

int main(int argc, char* argv[]) {
	
	//display instruction to testing
	if(argc != 6) {
		printf("Real time system test\n");
		printf("\n");
		printf("Usage:\n");
		printf("%s port_number frequency tolerance_time_in_ms single_message_size_in_bytes [zerocopy|nozerocopy]_for_zerocopy_sender.\n", argv[0]);
		exit(0);
	}
	
	//get info from user
	int port_number = atoi(argv[1]);
	int frequency = atoi(argv[2]);
	int tolerance_time = atoi(argv[3]);
	message_size = atoi(argv[4]);
	data_to_be_sent = malloc(message_size);
	bool zerocopy = strcmp(argv[5],"zerocopy")==0? true : false;
	
	//display info to user
	printf("Port Number : %d\n", port_number);
	printf("Frequency Used : %d\n", frequency);
	printf("Tolerance Time : %d\n", tolerance_time);
	printf("Actual Tolerance Time : %d\n", min(tolerance_time, (1000/frequency) - 5));
	printf("Singe Message Size : %d\n", message_size);
	printf("Zero-copy Mode : %s\n\n", zerocopy==true?"Enabled":"Not Enabled");
	
	Sender* sender;
	if(zerocopy) {
		sender = new TCPSenderZC(port_number);
	} else {
		sender = new TCPSender(port_number);
	}

	FreeWaitTimer timer;

	RealTimeSystem system;

	if(!system.set_timer(&timer)) {
		cout << "Failed to set the timer object." << endl;
		cout << system.get_error() << endl;
		return 1;
	}

	if(!system.set_sender(sender)) {
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

	if(!system.initialize()) {
		cout << "Failed to initialize the system." << endl;
		cout << system.get_error() << endl;
		return 5;
	}
	
	system.run();
	cout << "Failed to run the system." << endl;
	cout << system.get_error() << endl;
	delete sender;
	return 6;
}