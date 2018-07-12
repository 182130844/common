
#pragma once
#include <list>
#include <thread>
#include <mutex>

namespace shadow {
	class base_thread
	{
	public:
		base_thread(const char* name = "");
		virtual ~base_thread();
		virtual void thread_proc() = 0;
		bool activate(size_t threads = 1);
		void join();
		static void sleep(int ms);

	protected:
		char                    thread_name[128];
	private:
		void                    run();
		std::mutex              mutex_;
		std::list<std::thread>  thread_list_;
	};
}