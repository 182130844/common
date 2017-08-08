#include "base_thread.h"

base_thread::base_thread(const char* name) {
	strcpy(thread_name, name);

#ifndef _WIN32
	pthread_attr_init(&m_attr);
#endif
}

base_thread::~base_thread() {
#ifndef _WIN32
	pthread_attr_destroy(&m_attr);
#endif
}

bool base_thread::activate(int threads, int stack_size) {
	if (threads < 1) {
		return false;
	}
	if (stack_size < 0)
		stack_size = 0;

	for (int i = 0; i < threads; i++) {

#ifdef _WIN32	
		HANDLE hThread = _beginthreadex(NULL, stack_size, _internal_proc, this, 0, NULL);
		if (hThread == NULL) {
			printf("can't create thread (%d)\n", GetLastError());
			return false;
		}
		Sleep(10);
		m_thread_list.push_back(hThread);
#else
		pthread_t tid;
		int err = pthread_create(&tid, &m_attr, _internal_proc, this);
		if (err != 0) {
			printf("can't create thread (%s)\n", strerror(err));
			return false;
		}
		usleep(10000);
		m_thread_list.push_back(tid);
#endif
	}
	return true;
}

bool base_thread::kill_all() {
	guard g(&m_mutex);
	if (m_thread_list.empty())
		return false;

	for (ThreadIdIterator it = m_thread_list.begin(); it != m_thread_list.end(); it++) {
#ifdef _WIN32
		TerminateThread(*it, 0);
		CloseHandle(*it);
#else
		pthread_cancel(*it);
#endif
	}
	m_thread_list.clear();
	return true;
}

bool base_thread::wait_finish() {
	guard g(&m_mutex);
	if (m_thread_list.empty())
		return true;

	for (ThreadIdIterator it = m_thread_list.begin(); it != m_thread_list.end(); it++) {
#ifdef _WIN32
		int r = pthread_join(*it, 0);
		if (r != 0) {
			printf("pthread_join r=%d\n", r);
		}
#else
		DWORD r = WaitForSingleObject(*it, INFINITE);
		if (r != 0) {
			printf("WaitForSingleObject r=%d\n", r);
		}
#endif
	}
	m_thread_list.clear();
	return true;
}

#ifdef _WIN32
unsigned long base_thread::_internal_proc(void* ptr) {
#else
void*         base_thread::_internal_proc(void* ptr) {
#endif
	base_thread* pthis = reinterpret_cast<base_thread*>(ptr);
	
	char name[64] = { 0 };
	if (pthis != nullptr) {
		try {
			strcpy(name, pthis->thread_name);
			pthis->thread_proc();
		}
		catch (...) {
			printf("thread->thread_proc exception (%s)\n", name);
		}
	}
#ifdef _WIN32
	return 0;
#endif
}