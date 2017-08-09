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
using thread_id_list = list<HANDLE>;
#else
using thread_id_list = list<pthread_t>;
#endif

using thread_id_iterator = thread_id_list::iterator;

class base_thread
{
public:
	base_thread(const char* name);
	~base_thread();

	virtual int on_thread_proc() = 0;

	bool activate(int number_of_threads = 1, int stack_size = 4 * 1024 * 1024); // default stack size 4Mb
	bool kill_all();
	bool wait_finish();

protected:
	char thread_name[64];

private:

#ifdef _WIN32
	static unsigned int __stdcall _bt_proc(void* ptr);
#else
	static void*                  _bt_proc(void* ptr);
#endif
#ifndef _WIN32
	pthread_attr_t   m_attr;
#endif

	thread_mutex     m_mutex;
	
	thread_id_list   m_thread_id_list;

};
#endif // __base_thread_h__
