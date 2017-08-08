#ifndef __base_thread_h__
#define __base_thread_h__

#ifdef _WIN32
#include <windows.h>
#else
#include <pthread.h>
#include <unistd.h>
#endif

#include <list>
using std::list;

#include "mutex.h"

#ifdef _WIN32
using ThreadIdList = list<HANDLE>;
#else
using ThreadIdList = list<pthread_t>;
#endif

using ThreadIdIterator = ThreadIdList::iterator;

class base_thread
{
public:
	base_thread(const char* name);
	~base_thread();

	virtual int thread_proc() = 0;

	bool activate(int threads = 1, int stack_size = 4 * 1048576); // 默认栈大小为4MB
	bool kill_all();
	bool wait_finish();

protected:
	char thread_name[64];

private:

#ifdef _WIN32
	static unsigned long __stdcall _internal_proc(void* ptr);
#else
	static void*                   _internal_proc(void* ptr);
#endif
#ifndef _WIN32
	pthread_attr_t m_attr;
#endif

	thread_mutex   m_mutex;
	
	ThreadIdList   m_thread_list;

};
#endif // __base_thread_h__
