#include "../../includes/mutex_raii.h"

MutexRAII::MutexRAII(pthread_mutex_t& lock) : lock_(lock) {
	is_locked_ = false;
}
MutexRAII::~MutexRAII() {
	unlock();
}

void MutexRAII::lock_block() {
	is_locked_ = true;
	pthread_mutex_lock(&(lock_));
}

bool MutexRAII::lock_non_block() {
	if(pthread_mutex_trylock(&lock_) == 0) {
		is_locked_ = true;
		return true;
	}
	return false;
}

bool MutexRAII::unlock() {
	if(!is_locked_) {
		return false;
	}
	pthread_mutex_unlock(&lock_);
	is_locked_ = false;
	return true;
}