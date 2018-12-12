#ifndef SRC_UTILS_ATOMIC_VAL_RAII_H
#define SRC_UTILS_ATOMIC_VAL_RAII_H

#include <stdint.h>
#include <atomic>

/**
 * Class : AtomicValRAII
 * -------------------------------
 * This class will set the value for the atomic<T> to the end value
 * whenever it got destructed.
 */

template <class T>
class AtomicValRAII{

private:

	std::atomic<T>& prot_atomic_;
	T end_value_;
public:

	/**
	 * -------------------------------
	 * when the class get destructed, the prot_atomic object will be set to end_value.
	 */
	AtomicValRAII(std::atomic<T>& prot_atomic, T end_value);
	~AtomicValRAII();

};

template <class T>
AtomicValRAII<T>::AtomicValRAII(std::atomic<T>& prot_atomic, T end_value)
 : prot_atomic_(prot_atomic) , end_value_(end_value) {

 }

template <class T>
AtomicValRAII<T>::~AtomicValRAII() {
	prot_atomic_ = end_value_;
}

#endif