#pragma once
#include <chrono>
#include <thread>
#include <atomic>
#include <functional>
#include <condition_variable>

/**
 * @brief A wrapper class around condition variable
 * to help with threads synchronization
 */
class ThreadController {
public:
	ThreadController() : cv(), mutex(), shall_stop(false) { }

	/**
	 * @tparam Quantity - chrono based time quantity(i.e.std::chrono::seconds(value))
	 * @param time
	 */
	template <typename Quantity>
	bool wait_for(const Quantity& time);

	/**
	 * @brief Notifies all threads that they should be terminated.
	 */
	void kill();
private:
	std::condition_variable cv;
	std::mutex mutex;
	bool shall_stop;
};

template<typename Quantity>
bool ThreadController::wait_for(const Quantity& time) {
	std::unique_lock<std::mutex> lock(mutex);
	return !cv.wait_for(lock, time, [&] { return shall_stop; });
}

void ThreadController::kill() {
	std::unique_lock<std::mutex> lock(mutex);
	shall_stop = true;
	cv.notify_all();
}