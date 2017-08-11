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
	pthread_attr_init(&_bt_attr);
#endif
}

base_thread::~base_thread() {
#ifdef _WIN32
	// close handle
	for (auto it : _bt_thread_id_list) {
		CloseHandle(it);
	}
#else
	pthread_attr_destroy(&_bt_attr);
#endif
}

bool base_thread::activate(int number_of_threads, int stack_size) {
	if (number_of_threads < 1) {
		return false;
	}
	if (stack_size < 0)
		stack_size = 0;

	for (int i = 0; i < number_of_threads; i++) {

#ifdef _WIN32
		unsigned int tid = 0;
		HANDLE hThread = (HANDLE)_beginthreadex(NULL, stack_size, _bt_proc, this, 0, &tid);
		if (hThread == NULL) {
			printf("can't create thread (%d)\n", GetLastError());
			return false;
		}
		Sleep(10);
		_bt_thread_id_list.push_back(hThread);
#else
		pthread_t tid;
		int err = pthread_create(&tid, &_bt_attr, _bt_proc, this);
		if (err != 0) {
			printf("can't create thread (%s)\n", strerror(err));
			return false;
		}
		usleep(10000);
		_bt_thread_id_list.push_back(tid);
#endif
	}
	return true;
}

bool base_thread::kill_all() {
	auto_lock _al(&_bt_mutex);
	if (_bt_thread_id_list.empty())
		return false;

	for (thread_id_iterator it = _bt_thread_id_list.begin(); it != _bt_thread_id_list.end(); it++) {
#ifdef _WIN32
		TerminateThread(*it, 0);
		CloseHandle(*it);
#else
		pthread_cancel(*it);
#endif
	}
	_bt_thread_id_list.clear();
	return true;
}

bool base_thread::wait_finish() {
	auto_lock _al(&_bt_mutex);
	if (_bt_thread_id_list.empty())
		return true;

	for (thread_id_iterator it = _bt_thread_id_list.begin(); it != _bt_thread_id_list.end(); it++) {
#ifdef _WIN32
		DWORD r = WaitForSingleObject(*it, INFINITE);
		if (r != 0) {
			printf("WaitForSingleObject r=%d\n", r);
		}
#else
		int r = pthread_join(*it, 0);
		if (r != 0) {
			printf("pthread_join r=%d\n", r);
		}
#endif
	}
	_bt_thread_id_list.clear();
	return true;
}

#ifdef _WIN32
unsigned int base_thread::_bt_proc(void* ptr) {
#else
void*        base_thread::_bt_proc(void* ptr) {
#endif
	base_thread* pthis = reinterpret_cast<base_thread*>(ptr);
	
	char name[64] = { 0 };
	if (pthis != nullptr) {
		try {
			strcpy(name, pthis->thread_name);
			pthis->on_thread_proc();
		}
		catch (...) {
			printf("thread->thread_proc exception (%s)\n", name);
		}
	}
	return 0;
}