#include "../../includes/tcp_sender.h"

namespace tcp_sender{

	#define END_THREAD_ERROR(ERROR_FLAG, ERROR_CODE)\
		shared_data->is_error = (ERROR_FLAG);	\
		shared_data->error_code = (ERROR_CODE);	\
		prot_lock.~MutexRAII(); 		\
		prot_term_flag.~AtomicValRAII();	\
		pthread_exit(NULL)						

	enum TCP_SENDER_ERROR_CODES
	 {CANT_CPU_AFFINITY, CANT_SOCKET_TIMEOUT, SENDING_ERROR,
	  NOT_SUPPORTED_DATA_TYPE, NO_ERROR, CANT_SOCKET_BUFFERS};
}

using namespace tcp_sender;
using namespace timers_utils;

static void* tcp_worker_function(void* data) {
	//the shared data between the main thread and the worker thread.
	SenderWorkerData* shared_data = (SenderWorkerData*) data;
	//I'm alive!
	shared_data->is_terminated_thread = false;
	shared_data->thread_initialized = true;
	//release the lock whenever the scope of this function end
	MutexRAII prot_lock(shared_data->packets_mutex);
	//set the termination flag of this thread to true whenever this
	//function goes out of scope.
	AtomicValRAII<bool> prot_term_flag(shared_data->is_terminated_thread, true);
	//this is the buffer to copy data from the shared buffer to be sent later.
	DataPacketsList data_to_be_sent(0);
	//stick the thread to core 0
	cpu_set_t cpuset;
	CPU_ZERO(&cpuset);
	CPU_SET(CPU_CORE_AFFINITY, &cpuset);
	if(pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset) != 0) {
		END_THREAD_ERROR(true, CANT_CPU_AFFINITY);
	}
	//set the timeout to the socket to 1 second
	struct timeval timeout;
	timeout.tv_sec = 1;
	timeout.tv_usec = 0;
	if(setsockopt(shared_data->sock_fd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(struct timeval)) < 0) {
		END_THREAD_ERROR(true, CANT_SOCKET_TIMEOUT);
	}
	//set buffer sizes for send and recv
	int buff_size = TCP_SENDER_SEND_BUFFER;
	if(setsockopt(shared_data->sock_fd, SOL_SOCKET, SO_SNDBUF, &buff_size, sizeof(buff_size)) < 0) {
		END_THREAD_ERROR(true, CANT_SOCKET_BUFFERS);
	}
	buff_size = TCP_SENDER_RECV_BUFFER;
	if(setsockopt(shared_data->sock_fd, SOL_SOCKET, SO_RCVBUF, &buff_size, sizeof(buff_size)) < 0) {
		END_THREAD_ERROR(true, CANT_SOCKET_BUFFERS);
	}
	//start the sending loop
	while(!shared_data->terminate_thread) {
		/** 
		 * copy all the data from the buffer if available
		 **/
		//acquire the lock
		prot_lock.lock_block();

		//check if there is no data
		while(shared_data->packets_list == NULL) {
			//check if the signal due to the termination
			if(shared_data->terminate_thread) {
				END_THREAD_ERROR(false, NO_ERROR);
			}
			//if no data yet and no termination signal arrived, then wait on the condition
			pthread_cond_wait(&(shared_data->packets_cond), &(shared_data->packets_mutex));
		}
		//copy the data to the buffer to be sent later
		data_to_be_sent = *(shared_data->packets_list);
		shared_data->packets_list = NULL;
		//release the lock
		prot_lock.unlock();
		/** 
		 * send all the data from the buffer list
		 **/
		for(int i=0; i<data_to_be_sent.num_packets; i++) {
			//get the packet to be sent
			DataPacket* current_packet = (data_to_be_sent.packets) + i;
			if (current_packet->data_ptr_type == DataPacket::DATA_PTR_MEMORY_LOCATION) {
				//send the packet from memory location
				uint_fast32_t data_sent = current_packet->data_offset, remaining_data = current_packet->data_size;
				while(remaining_data != 0) {
					//try to send
					ssize_t s = send(shared_data->sock_fd, ((char*)current_packet->data_ptr) + data_sent, remaining_data, 0);
					//detect error
					if(s < 0) {
						END_THREAD_ERROR(true, SENDING_ERROR);
					}
					//check thread termination signal
					if(shared_data->terminate_thread) {
						END_THREAD_ERROR(false, NO_ERROR);
					}
					//update state variables
					data_sent += s;
					remaining_data -= s;
				}
			} else if(current_packet->data_ptr_type == DataPacket::DATA_PTR_FILE_DESCRIPTOR) {
				//send the packet from file descriptor
				uint_fast32_t remaining_data = current_packet->data_size;
				off_t offset = current_packet->data_offset;
				while(remaining_data != 0) {
					//try to send
					ssize_t s = sendfile(shared_data->sock_fd, *((int*)current_packet->data_ptr), &offset, remaining_data);
					//detect error
					if(s < 0) {
						END_THREAD_ERROR(true, SENDING_ERROR);
					}
					//check thread termination signal
					if(shared_data->terminate_thread) {
						END_THREAD_ERROR(false, NO_ERROR);
					}
					//update state variables
					remaining_data -= s;
				}
			} else {
				END_THREAD_ERROR(true, NOT_SUPPORTED_DATA_TYPE);
			}

		}

		//mark the send as done
		shared_data->is_done = true;
	}
	
	END_THREAD_ERROR(false, NO_ERROR);
}


TCPSender::TCPSender(uint_fast16_t port) : error_handler_("TCPSender") {
	port_ = port;
	server_sock_fd_ = -1;
	client_sock_fd_ = -1;
	initialized_ = false;
	//this is true here to prevent deadlock in case the object got
	//destroyed before initialization, this flag means that there is
	//no thread currently operate and will be set to false if the 
	//thread was running.
	shared_data_.is_terminated_thread = true;
}

bool TCPSender::create_server() {

    struct addrinfo hints, *servinfo, *p;
    int yes=1;
    int rv;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo(NULL, std::to_string(port_).c_str(), &hints, &servinfo)) != 0) {
		error_handler_.set_error(std::string("getaddrinfo: ") + gai_strerror(rv));
        return false;
    }

    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((server_sock_fd_ = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            continue;
        }
        if (setsockopt(server_sock_fd_, SOL_SOCKET, SO_REUSEADDR, &yes,
                sizeof(int)) == -1) {
			error_handler_.set_error("error in setsockopt");
            return false;
        }
        if (bind(server_sock_fd_, p->ai_addr, p->ai_addrlen) == -1) {
            close(server_sock_fd_);
            continue;
        }
        break;
    }

    freeaddrinfo(servinfo); // all done with this structure

    if (p == NULL)  {
		error_handler_.set_error("server: failed to bind");
        return false;
    }

    if (listen(server_sock_fd_, 1) == -1) {
		error_handler_.set_error("error in listen to the port");
        return false;
    }

    //ignore SIGPIPE
    signal(SIGPIPE, SIG_IGN);


    return true;
}

bool TCPSender::get_client() {

    struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size;

    client_sock_fd_ = accept(server_sock_fd_, (struct sockaddr *)&their_addr, &sin_size);
    if (client_sock_fd_ == -1) {
		error_handler_.set_error("error in accepting the client");
        return false;
    }

    return true;
}


bool TCPSender::initialize() {
	
	//clean the last state
	//destroying the thread
	//closing open files
	end_sender();

	//re initialize the shared data
	shared_data_.packets_list = NULL;
	shared_data_.is_done = true;
	shared_data_.is_error = false;
	shared_data_.sock_fd = -1;
	shared_data_.terminate_thread = false;
	shared_data_.thread_initialized = false;

	//again this can't be initialized here and must be initialized in
	//the worker thread itself to prevent deadlock.
	//shared_data_.is_terminated_thread = false;

	shared_data_.error_code = 0;
	shared_data_.packets_mutex = PTHREAD_MUTEX_INITIALIZER;
	shared_data_.packets_cond = PTHREAD_COND_INITIALIZER;

	//create the server
	if(!create_server()) {
		return false;
	}
	
	//wait for client to connect
	if(!get_client()) {
		return false;
	}

	//shutdown the server
	shutdown(server_sock_fd_, SHUT_RDWR);
	close(server_sock_fd_);
	server_sock_fd_ = -1;

	//set the client fd in the shared data
	shared_data_.sock_fd = client_sock_fd_;

	//create thread
	struct sched_param param;
	pthread_attr_t attr;
	int ret;

	/* Initialize pthread attributes (default values) */
	ret = pthread_attr_init(&attr);
	if (ret) {
		error_handler_.set_error("init pthread attributes failed");
		return false;
	}

	/* Set a specific stack size  */
	ret = pthread_attr_setstacksize(&attr, PTHREAD_STACK_MIN);
	if (ret) {
		error_handler_.set_error("pthread setstacksize failed");
		return false;
	}

	/* Set scheduler policy and priority of pthread */
	ret = pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
	if (ret) {
		error_handler_.set_error("pthread setschedpolicy failed");
		return false;
	}
	param.sched_priority = 98;
	ret = pthread_attr_setschedparam(&attr, &param);
	if (ret) {
		error_handler_.set_error("pthread setschedparam failed");
		return false;
	}

	/* Use scheduling parameters of attr */
	ret = pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
	if (ret) {
		error_handler_.set_error("pthread setinheritsched failed");
		return false;
	}

	int th_st = pthread_create(&worker_thread_, &attr, tcp_worker_function, (void*) &shared_data_);
	if(th_st) {
		error_handler_.set_error("pthread_create() return code: " + std::to_string(th_st));
		return false;
	}

	//wait until the thread initialized
	while(!shared_data_.thread_initialized) {
		milliseconds_sleep(5);
	}

	initialized_ = true;
	return true;
}

bool TCPSender::send(DataPacketsList* list) {

	//if the object not yet initialized
	if(!initialized_) {
		error_handler_.set_error("You must initialize the sender object first");
		return false;
	}

	//if the sender worker thread terminates, get the error from it
	if(shared_data_.is_terminated_thread) {
		if(!shared_data_.is_error) {
			error_handler_.set_error("No Thread Available to execute the send operation");
			return false;
		}
		switch(shared_data_.error_code) {
			case CANT_CPU_AFFINITY:
				error_handler_.set_error("Sender worker thread can't set CPU Affinity.");
			break;
			case CANT_SOCKET_TIMEOUT:
				error_handler_.set_error("Sender worker thread can't set timeout on the socket.");
			break;
			case SENDING_ERROR:
				error_handler_.set_error("Sender worker thread, error while sending the data. ");
			break;
			case NOT_SUPPORTED_DATA_TYPE:
				error_handler_.set_error("Sender worker thread can't send that type of packets.");
			break;
			case CANT_SOCKET_BUFFERS:
				error_handler_.set_error("Can't reserve the buffers needed for the socket.");
			break;
			default:
				error_handler_.set_error("Unknown error in sender worker thread.");
			break;
		}
		return false;
	}

	//if there is already send operation, then decline this send operation
	if(!shared_data_.is_done) {
		error_handler_.set_error("system called send(DataPacketsList*) while on going send operation.");
		return false;
	}

	//release the lock whenever the scope of this function end
	MutexRAII prot_lock(shared_data_.packets_mutex);

	//add the data, set the is_done flag, and signal the thread
	prot_lock.lock_block();
	shared_data_.packets_list = list;
	shared_data_.is_done = false;
	pthread_cond_signal(&(shared_data_.packets_cond));
	prot_lock.unlock();
	return true;

}

bool TCPSender::is_send_done() {
	return shared_data_.is_done;
}

bool TCPSender::end_sender() {

	//mark it as uninitialized
	initialized_ = false;

	//end the thread if it was running
	if(!shared_data_.is_terminated_thread) {
		//terminate the worker thread
		shared_data_.terminate_thread = true;
		//signal the thread to terminate
		MutexRAII prot_lock(shared_data_.packets_mutex);
		prot_lock.lock_block();
		shared_data_.packets_list = NULL;
		shared_data_.is_done = false;
		pthread_cond_signal(&(shared_data_.packets_cond));
		prot_lock.unlock();
		//wait until the thread terminates
		while(!shared_data_.is_terminated_thread) {
			milliseconds_sleep(50);
		}
	}

	//close the sockets
	//close open files
	if(server_sock_fd_ != -1) {
		shutdown(server_sock_fd_, SHUT_RDWR);
		close(server_sock_fd_);
		server_sock_fd_ = -1;
	}
	if(client_sock_fd_ != -1) {
		shutdown(client_sock_fd_, SHUT_RDWR);
		close(client_sock_fd_);
		client_sock_fd_ = -1;
	}
	return true;
}


std::string TCPSender::get_error() {
	std::string error = error_handler_.get_error();
	error_handler_.clear_error();
	return error;
}

bool TCPSender::is_error() {
	return error_handler_.is_error();
}

TCPSender::~TCPSender() {
	end_sender();
}
