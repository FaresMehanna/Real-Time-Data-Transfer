#ifndef SRC_UTILS_MUTEX_RAII_H
#define SRC_UTILS_MUTEX_RAII_H

#include <stdint.h>
#include <pthread.h>

/**
 * Class : LockRAII
 * -------------------------------
 * This class is wrapper around mutex lock to protected it against
 * mistakes.
 */
class MutexRAII{

private:

	bool is_locked_;
	pthread_mutex_t& lock_;

public:

	MutexRAII(pthread_mutex_t& lock);
	~MutexRAII();

	/**
	 * Method : lock_block
	 * -------------------------------
	 * tries to acquire the mutex, block until the mutex become available.
	 */
	void lock_block();

	/**
	 * Method : lock_non_block
	 * -------------------------------
	 * tries to acquire the mutex, if success true returns,
	 * false if failed to acquire the mutex.
	 */
	bool lock_non_block();	

	/**
	 * Method : unlock
	 * -------------------------------
	 * unlock the mutex only if the mutex have been locked before.
	 */
	bool unlock();

};

#endif