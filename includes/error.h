#ifndef SRC_UTILS_ERROR_H_
#define SRC_UTILS_ERROR_H_

#include <string>
#include <stdint.h>

/**
 * Class : Error
 * -------------------------------
 * This class handles errors.
 */
class Error{

private:

	bool is_error_;
	std::string error_msg_;
	std::string owner_identifier_;

public:

	Error(std::string owner_identifier);
	~Error();

	/**
	 * Method : set_error
	 * -------------------------------
	 * set the error and remove the last one if existed.
	 */
	void set_error(std::string error_msg);
	//void set_error(char* error_msg);

	/**
	 * Method : get_error
	 * -------------------------------
	 * @return the error string if an error occurs, empty string will
	 * be returned if no error existed.
	 */
	std::string get_error();

	/**
	 * Method : is_error
	 * -------------------------------
	 * Check if an error existed.
	 * @return true if an error occurs, false otherwise.
	 */
	bool is_error();

	/**
	 * Method : clear_error
	 * -------------------------------
	 * clear the error if one existed.
	 */
	void clear_error();
	
};

#endif