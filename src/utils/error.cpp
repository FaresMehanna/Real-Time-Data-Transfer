#include "../../includes/error.h"

using namespace std;

Error::Error(string owner_identifier) {
	is_error_ = false;
	error_msg_ = "";
	owner_identifier_ = owner_identifier;
}

Error::~Error() {

}

void Error::set_error(string error_msg) {
	error_msg_ = error_msg;
	is_error_ = true;
}

string Error::get_error() {
	return error_msg_;
}

bool Error::is_error() {
	return is_error_;
}

void Error::clear_error() {
	is_error_ = false;
	error_msg_ = "";
}