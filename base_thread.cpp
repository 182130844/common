#include "base_thread.h"
#include <stdio.h>
#include <stdlib.h>
#ifdef _WIN32
#include <process.h>
#else
#include <string.h>
#include <sys/types.h>
#endif

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

bool base_thread::create(int number_of_threads, int stack_size) {
	if (number_of_threads < 1) {
		return false;
	}
	if (stack_size < 0)
		stack_size = 0;

	for (int i = 0; i < number_of_threads; i++) {

#ifdef _WIN32	
		HANDLE hThread = _beginthreadex(NULL, stack_size, _internal_proc, this, 0, NULL);
		if (hThread == NULL) {
			printf("can't create thread (%d)\n", GetLastError());
			return false;
		}
		Sleep(10);
		m_thread_id_list.push_back(hThread);
#else
		pthread_t tid;
		int err = pthread_create(&tid, &m_attr, _internal_proc, this);
		if (err != 0) {
			printf("can't create thread (%s)\n", strerror(err));
			return false;
		}
		usleep(10000);
		m_thread_id_list.push_back(tid);
#endif
	}
	return true;
}

bool base_thread::kill_all() {
	auto_lock _al(&m_mutex);
	if (m_thread_id_list.empty())
		return false;

	for (thread_id_iterator it = m_thread_id_list.begin(); it != m_thread_id_list.end(); it++) {
#ifdef _WIN32
		TerminateThread(*it, 0);
		CloseHandle(*it);
#else
		pthread_cancel(*it);
#endif
	}
	m_thread_id_list.clear();
	return true;
}

bool base_thread::wait_finish() {
	auto_lock _al(&m_mutex);
	if (m_thread_id_list.empty())
		return true;

	for (thread_id_iterator it = m_thread_id_list.begin(); it != m_thread_id_list.end(); it++) {
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
	m_thread_id_list.clear();
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