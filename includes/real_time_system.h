#ifndef SRC_SYSTEM_REAL_TIME_SYSTEM_H
#define SRC_SYSTEM_REAL_TIME_SYSTEM_H

#include <sched.h>
#include <sys/mman.h>

#include <string>
#include <algorithm>
#include <iostream>

#include "flags.h"
#include "error.h"
#include "data_packets.h"
#include "timer.h"
#include "sender.h"
#include "timers_utils.h"

class RealTimeInfo;

/**
 * Class : RealTimeSystem
 * -------------------------------
 * This class will run the system by connecting the timer, sender and the
 * user application all together.
 */

class RealTimeSystem{

private:
	//timer to provide accurate sleep & wake
	Timer* timer_;
	//sender to transmit the data
	Sender* sender_;
	//the user application function to provide the data to be sent
	DataPacketsList (*user_app_func_)(RealTimeInfo*);
	//error handler class
	Error error_handler_;
	//system frequency
	uint_fast16_t frequency_;
	//allow skip mode
	bool allow_skip_mode_;
	//is the object initialized
	bool initialized_;
	//time tolerance of the system
	uint_fast16_t ms_tolerance_;
	//method to handle converting the thread to real time thread with high priority
	bool convert_rt_thread();
public:


	/**
	 * Method : constructor
	 * -------------------------------
	 */
	RealTimeSystem();


	/**
	 * Method : set_timer
	 * -------------------------------
	 * The method will set the timer object with which the system will sleep.
	 * @param timer is the pointer to constructed timer object.
	 * @return true if successful, false otherwise.
	 */
	bool set_timer(Timer* timer);

	/**
	 * Method : set_sender
	 * -------------------------------
	 * The method will set the sender object with which the system will send the data.
	 * @param sender is the pointer to constructed sender object.
	 * @return true if successful, false otherwise.
	 */
	bool set_sender(Sender* sender);

	/**
	 * Method : set_sender
	 * -------------------------------
	 * The method will set the user provided function which will provide data
	 * to be sent.
	 * @param user_app_func is the pointer to the function which will return.
	 * DataPacketsList to be sent. 
	 * @return true if successful, false otherwise.
	 */
	bool set_user_data_fn(DataPacketsList (*user_app_func)(RealTimeInfo*));


	/**
	 * Method : set_frequency
	 * -------------------------------
	 * The method will set the system frequency to the given one.
	 * @param frequency is the frequency with which the system will run.
	 * @return true if the frequency is supported by the timer.
	 * false otherwise.
	 * NOTE that, the checking for frequency validation may be deferred
	 * to initialize(0) method.
	 */
	bool set_frequency(uint_fast16_t frequency);


	/**
	 * Method : set_system_ms_tolerance
	 * -------------------------------
	 * The method will set the tolerated ms for each time slice.
	 * @param ms_tolerance is the amount of time in milliseconds where the system can give
	 * to the sender to complete the current packet only if the sender couldn't complete
	 * sending the data. Note that the data must be available during that extra time.
	 * the system might decide to ignore/change this value.
	 * so the ms_tolerance consider the upper limit only.
	 * by default ms_tolerance in the system = 0.
	 */
	void set_ms_tolerance(uint_fast16_t ms_tolerance);

	/**
	 * Method : skip_mode
	 * -------------------------------
	 * The method will set the ability to skip the next data packet only if the
	 * current data packet not sent yet.
	 * @param skip_mode is a flag to determine what should happen if the sender couldn't 
	 * send the packet, if the skip_mode=true, then if the current packet not sent yet, the
	 * next packet will be skipped, if it is false, then the transmission will be stopped if
	 * single packet couldn't be delivered.
	 */
	void skip_mode(bool allow_skip_mode);

	/**
	 * Method : initialize
	 * -------------------------------
	 * The method will initialize the timer and sender.
	 * this method must be called before run(0) and after setting all the needed
	 * data in the object.
	 * if everything goes fine the function should return true.
	 * if error occur, the function will return false.
	 */
	bool initialize();

	/**
	 * Method : send_before_run
	 * -------------------------------
	 * The method will be used if the protocol require sending a message first
	 * before the sequence of data starting to transmit, the user will provide a
	 * list of packet and a time limit for the sending. 
	 * This function is optional, but if you will call it, you must call it after 
	 * calling initialize(0).
	 * time_limit_ms is the milliseconds available to execute this order.
	 * you should give good estimate to the actual transmuting + safe margin.
	 * if the transmission done successfully true will be returned and the user
	 * can run the system and start sending the data sequences.
	 * if the sending didn't done successfully, the system will shutdown the sender and
	 * the user must re initialize(0) the real_time_system again which in turn will
	 * re initialize each component again.
	 */
	bool send_before_run(DataPacketsList list_of_data, uint_fast32_t time_limit_ms);

	/**
	 * Method : run
	 * -------------------------------
	 * The method will start the system in the exact time of calling.
	 * if everything goes fine the function should never return unless the user 
	 * used stop_system().
	 * if error occur at first or while transmitting the data the function will return
	 * false and the error will be set to be pulled by is_error & get_error
	 */
	bool run();

	/**
	 * Method : get_error
	 * -------------------------------
	 * @return the error string if an error occurs and the error will be removed
	 * after this call, if no error occurs, it return empty string ("").
	 */
	std::string get_error();

	/**
	 * Method : is_error
	 * -------------------------------
	 * Check if an error existed.
	 * @return true if an error occurs, false otherwise.
	 */
	bool is_error();

	~RealTimeSystem();

};

/**
 * Class : RealTimeInfo
 * -------------------------------
 * This class will represent the data shared between the system and the user
 * provided function.
 * this class will contain several data that might be interesting to the
 * user. as well as the option to stop the system.
 */

class RealTimeInfo{
	private:
		uint_fast32_t sequence_number_;
		bool skip_next_data_;
		uint_fast16_t delayed_ms_;
		bool system_stopped_;

	public:

		/**
		 * Method : Constructor
		 * -------------------------------
		 * set the class data.
		 */		
		RealTimeInfo(uint_fast32_t sequence_number, bool skip_next_data, uint_fast16_t delayed_ms, bool system_stopped);

		/**
		 * Method : get_sequence_number
		 * -------------------------------
		 * return the number of the current call to the user function.
		 */
		uint_fast32_t get_sequence_number();
		/**
		 * Method : get_delayed_time_ms
		 * -------------------------------
		 * return the number of milliseconds which used to complete the last
		 * send from this time slot.
		 * this option only used if the ms_tolerance was used when initializing
		 * the system.
		 */
		uint_fast32_t get_delayed_time_ms();
		/**
		 * Method : is_skipped_data
		 * -------------------------------
		 * return true if this data will be ignored due to the flag skip_mode been
		 * set to true when initializing the system.
		 */
		bool is_skipped_data();
		/**
		 * Method : stop_system
		 * -------------------------------
		 * the user will call this function only if the user want to stop the system.
		 */
		void stop_system();
		/**
		 * Method : is_system_stopped
		 * -------------------------------
		 * return true if the user decided to stop the system.
		 */
		bool is_system_stopped();
};


#endif