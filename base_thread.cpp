#include <iostream>
#include <chrono>
#include "base_thread.h"
#pragma warning(disable:4996)

base_thread::base_thread(const char* name) {
	strcpy(thread_name, name);
}

base_thread::~base_thread() {

}

bool base_thread::activate(size_t threads) {
	std::lock_guard<std::mutex> lock(mutex_);
	for (size_t i = 0; i < threads; i++) {
		thread_list_.emplace_back(&base_thread::run, this);
	}
	return true;
}

void base_thread::join() {
	for (auto& thread : thread_list_) {
		if (thread.joinable()) {
			thread.join();
		}
	}
}

void base_thread::run() {
	try {
		thread_proc();
	}
	catch (...) {
		std::cout << "thread_proc exception: " << thread_name << std::endl;
	}
}

void base_thread::sleep(int ms)
{
	std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}