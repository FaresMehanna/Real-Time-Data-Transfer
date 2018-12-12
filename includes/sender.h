#ifndef SRC_SENDERS_SENDER_H
#define SRC_SENDERS_SENDER_H

#include <string>
#include <stdint.h>

#include "data_packets.h"

/**
 * Pure Abstract Class : Sender
 * -------------------------------
 * This abstract class MUST be inherited by each sender class.
 * Each derived class MUST implement ALL the methods defined in this
 * class.
 */
class Sender{

public:

	/**
	 * Method : initialize
	 * -------------------------------
	 * The method should try to initialize the sender.
	 * This will be the first method to be called from the system after 
	 * constructing the object.
	 * @return true if the sender correctly initialized, false in case of any
	 * error during the initialization.
	 * in case of false the method MUST report the error.
	 */
	virtual bool initialize() = 0;

	/**
	 * Method : send
	 * -------------------------------
	 * The method should send the data in the buffer.
	 * The call MUST NOT be blocking and MUST return in very short amount of
	 * time. If the method failed to return before the next tick in the timer
	 * it will stop the system.
	 * @param list is a pointer to DataPacketsList to be sent.
	 * @return true if the send operation can be executed, false otherwise,
	 * in case of false the method MUST report the error.
	 */
	virtual bool send(DataPacketsList* list) = 0;

	/**
	 * Method : is_send_done
	 * -------------------------------
	 * The method check if the last call to send(2) done correctly.
	 * @return true if the send operation done successfully before this call,
	 * false if the send operation not done yet.
	 */
	virtual bool is_send_done() = 0;

	/**
	 * Method : end_sender
	 * -------------------------------
	 * The method close the sender and clean all its resources.
	 * @return true if the end operation done successfully,
	 * false otherwise, in case of false the method MUST report the error.
	 */
	virtual bool end_sender() = 0;

	/**
	 * Method : get_error
	 * -------------------------------
	 * @return the error string if an error occurs and the error MUST be removed
	 * after this call, if no error occurs, return empty string ("").
	 */
	virtual std::string get_error() = 0;

	/**
	 * Method : is_error
	 * -------------------------------
	 * Check if an error existed.
	 * @return true if an error occurs, false otherwise.
	 */
	virtual bool is_error() = 0;

	virtual ~Sender() {};

};

#endif