#ifndef SRC_UTILS_DATA_PACKETS_H
#define SRC_UTILS_DATA_PACKETS_H

#include <string.h>
#include <stdint.h>
#include <pthread.h>

#include <iostream>
#include <list>
#include <atomic>

/**
 * Struct : DataPacket
 * -------------------------------
 * This struct will hold the needed data for data transmission.
 */
struct DataPacket{

	//type of the data
	enum DATA_PTR_TYPE {DATA_PTR_MEMORY_LOCATION, DATA_PTR_FILE_DESCRIPTOR};
	DATA_PTR_TYPE data_ptr_type = DATA_PTR_MEMORY_LOCATION;

	//pointer to the starting address of the data
	void* data_ptr = NULL;

	//size of the data
	uint_fast32_t data_size = 0;

	//offset of the data
	uint_fast32_t data_offset = 0;
};

/**
 * Struct : DataPacket
 * -------------------------------
 * This struct will hold an array of DataPacket to allow several
 * transmission without copying them in single buffer.
 */
struct DataPacketsList{
	
	//array of packets to be sent
	DataPacket* packets = NULL;

	//number of packets in the array
	uint_fast32_t num_packets = 0;

	//constructor
	DataPacketsList(uint_fast32_t num_packets_) {
		//set number of packets
		num_packets = num_packets_;
		//allocate the array
		packets = (DataPacket*) malloc(num_packets * sizeof(DataPacket));
		//unrecoverable error
		if(NULL == packets) {
			printf("Memory error while allocating DataPacket.");
			exit(1);
		}
	}
	//destructor
	~DataPacketsList() {
		if(NULL != packets) {
			free(packets);
			packets = NULL;
		}
	}
	//copy constructor
	DataPacketsList(const DataPacketsList &obj) {
		//set number of packets
		num_packets = obj.num_packets;
		//allocate the array
		packets = (DataPacket*) malloc(num_packets * sizeof(DataPacket));
		//unrecoverable error
		if(NULL == packets) {
			printf("Memory error while allocating DataPacket.");
			exit(1);
		}
		memcpy(packets, obj.packets, obj.num_packets * sizeof(DataPacket));
	}
	//copy assignment operator
	DataPacketsList& operator=(const DataPacketsList &obj) {
		//free old data if found
		if(NULL != packets) {
			free(packets);
			packets = NULL;
		}
		/* copy new data */
		//set number of packets
		num_packets = obj.num_packets;
		//allocate the array
		packets = (DataPacket*) malloc(num_packets * sizeof(DataPacket));
		//unrecoverable error
		if(NULL == packets) {
			printf("Memory error while allocating DataPacket.");
			exit(1);
		}
		memcpy(packets, obj.packets, obj.num_packets * sizeof(DataPacket));
		return *this;
	}

};

/**
 * Struct : SenderWorkerData
 * -------------------------------
 * This struct will be used between the senders and their worker
 * threads/s.
 */
struct SenderWorkerData{

	//the data we got from the user to be sent
	DataPacketsList* packets_list = NULL;

	//is the worker thread finished sending?
	//this flag set by both the sender and the worker thread
	std::atomic<bool> is_done = {true};

	//is there an error in worker thread, only error thread change this
	//variable.
	std::atomic<bool> is_error = {false};

	//the sock_fd for the client which the sender will try to send data to.
	std::atomic<int> sock_fd = {-1};

	//send a termination signal to the thread
	//only main thread set this variable
	std::atomic<bool> terminate_thread = {false};
	
	//this flag set by the thread after it's initilization
	//we can't get or send any data to the thread untill this flag
	//becomes true
	std::atomic<bool> thread_initialized = {false};

	//the thread will response in this variable declaring it's termination
	//only worker thread will set this variable
	//can be cleared by the main thread when read
	std::atomic<bool> is_terminated_thread = {true};

	//to specify what error happen in the worker thread
	//only worker thread will set this variable
	//can be cleared by the main thread when read
	std::atomic<uint_fast8_t> error_code = {0};

	//mutex to synchronize the access to the list.
	//condition variable to signal the worker thread when new packets
	//need to be delivered.
	pthread_mutex_t packets_mutex = PTHREAD_MUTEX_INITIALIZER;
	pthread_cond_t packets_cond = PTHREAD_COND_INITIALIZER;
};

#endif