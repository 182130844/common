#include <iostream>
#include <chrono>
#include <string.h>
#include "base_thread.h"
#include <thread>

namespace shadow {
	base_thread::base_thread(const char* name) {
		strcpy(thread_name, name);

#ifndef _WIN32
		pthread_attr_init(&attr_);
#endif
	}

	base_thread::~base_thread() {
#ifndef _WIN32
		pthread_attr_destroy(&attr_);
#endif
		thread_list_.clear();
	}

	bool base_thread::activate(size_t threads) {
		std::lock_guard<std::mutex> lock(mutex_);
		for (size_t i = 0; i < threads; i++) {
#ifdef _WIN32
			unsigned int dwThreadID = 0;
			HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, run, this, 0, &dwThreadID);
			if (hThread == 0) {
				printf("can't create thread");
				return false;
			}
			thread_list_.emplace_back(hThread);
#else
			pthread_t tid;
			int err = pthread_create(&tid, &attr_, run, this);
			if (err != 0) {
				printf("can't create thread: %s\n", strerror(err));
				return false;
			}
			thread_list_.emplace_back(tid);
#endif
		}
		return true;
	}

	void base_thread::join() {
#ifndef _WIN32
		std::lock_guard<std::mutex> g(mutex_);
		for (auto& thread : thread_list_) {
			pthread_join(thread, 0);
		}
		thread_list_.clear();
#endif
	}

	bool base_thread::kill_all()
	{
		std::lock_guard<std::mutex> g(mutex_);
		for (auto& thread : thread_list_) {
#ifdef _WIN32
			TerminateThread(thread, 0);
#else
			pthread_cancel(thread);
#endif
		}
		thread_list_.clear();
		return true;
	}

#ifdef _WIN32
	unsigned int base_thread::run(void* param) {
#else
	void* base_thread::run(void* param) {
#endif
		base_thread* pthis = reinterpret_cast<base_thread*>(param);
		try {
			pthis->thread_proc();
		}
		catch (...) {
			std::cout << "thread_proc exception: " << "[" << std::this_thread::get_id() << "]" << pthis->thread_name << std::endl;
		}
		return 0;
	}
}