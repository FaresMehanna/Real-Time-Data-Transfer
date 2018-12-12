#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../includes/senders.h"
#include "../../includes/systems.h"
#include "../../includes/timers.h"

using namespace std;
using namespace timers_utils;

int main(void) {

	TCPSender sender(7575);

	if(!sender.initialize()) {
		cout << "Failed to initialize the TCPSender." << endl;
		cout << sender.get_error() << endl;
	}
	
	//first message
	string f_message = "Fares\n";
	string s_message = "This is the body\n";

	DataPacket m1;
	m1.data_ptr = (void *) f_message.c_str();
	m1.data_size = f_message.length();

	DataPacket m2;
	m2.data_ptr = (void *) s_message.c_str();
	m2.data_size = s_message.length();	

	while(1) {
		while(!sender.is_send_done()) {
			milliseconds_sleep(100);
		}
		DataPacketsList list(2);
		(list.packets)[0] = m1;
		(list.packets)[1] = m2;

		cout << sender.send(&list) << endl;
	}


}