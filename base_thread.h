
#pragma once
#include <list>
#include <mutex>
#ifdef _WIN32
#include <Windows.h>
#include <process.h>
#else
#include <pthread.h>
#endif
//
// compile link:
// linux: pthread
// windows: _CRT_SECURE_NO_WARNINGS
//

namespace shadow {
	class base_thread
	{
	public:
		base_thread(const char* name = "");
		virtual ~base_thread();
		virtual void thread_proc() = 0;
		bool activate(size_t threads = 1);
		void join();
		bool kill_all();
	protected:
		char                    thread_name[64];
	private:
#ifdef _WIN32
		static unsigned int __stdcall run(void* param);
#else
		static void* run(void* param);
#endif
		std::mutex              mutex_;
#ifndef _WIN32
		pthread_attr_t attr_;
		std::list<pthread_t>  thread_list_;
#else
		std::list<HANDLE>  thread_list_;

#endif
	};
}