#ifndef SRC_SENDERS_TCP_SENDER_H_
#define SRC_SENDERS_TCP_SENDER_H_

#include <iostream>
#include <string>

#include <stdint.h>
#include <limits.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/sendfile.h>

#include "flags.h"
#include "sender.h"
#include "tcp_sender.h"
#include "error.h"
#include "data_packets.h"
#include "mutex_raii.h"
#include "atomic_val_raii.h"
#include "timers_utils.h"

class TCPSender : public Sender{

private:

	//The port with which we will start a server
	uint_fast16_t port_;

	//The socket file descriptor of the client to stream data to and the current server
	int server_sock_fd_;
	int client_sock_fd_;

	//The thread where the actual transmission will happen in
	pthread_t worker_thread_;

	//The data which is shared between this class and it's worker thread
	SenderWorkerData shared_data_;

	//error handler class
	Error error_handler_;

	//initialized?
	bool initialized_;

	//create the server and set server_sock_fd_ to the server fd
	//returns true if every thing runs correctly, false otherwise
	//when false is returned the error_handler_ will be set accordingly.
	bool create_server();

	//this function will wait for a client,
	//if the client succeeded to connect the client_sock_fd_ will be
	//set accordingly and true will be returned, else false is returned.
	bool get_client();

public:

	TCPSender(uint_fast16_t port);

	bool initialize() override;

	bool send(DataPacketsList* list) override;

	bool is_send_done() override;

	bool end_sender() override;

	std::string get_error() override;

	bool is_error() override;

	~TCPSender();

};

#endif