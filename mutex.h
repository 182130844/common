
#pragma once
#ifndef _WIN32
#include <pthread.h>
#include <unistd.h>
#else
#include <Windows.h>
#endif
#include <stdio.h>

class mutex {
public:
	virtual void lock() = 0;
	virtual void un_lock() = 0;
	virtual bool try_lock() = 0;
};


class guard {
public:
	guard(mutex* p_mutex)
	{
		_p_mutex = p_mutex;
		if (_p_mutex) {
			_p_mutex->lock();
		}
	}
	guard(mutex& m)
	{
		_p_mutex = &m;
		_p_mutex->lock();
	}
	~guard()
	{
		if (_p_mutex) {
			_p_mutex->un_lock();
		}
	}
private:
	mutex* volatile _p_mutex;
};

#ifndef _WIN32
class thread_mutex : public mutex {
public:
	thread_mutex()
	{
		pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
		pthread_mutex_init(&_mutex, &attr);
	}
	~thread_mutex()
	{
		pthread_mutex_destroy(&_mutex);
	}
	virtual void lock()
	{
		pthread_mutex_lock(&_mutex);
	}
	virtual void un_lock()
	{
		pthread_mutex_unlock(&_mutex);
	}
	virtual bool try_lock()
	{
		return (pthread_mutex_trylock(&_mutex) == 0);
	}
private:
	pthread_mutexattr_t attr;
	pthread_mutex_t _mutex;
};
#else
class thread_mutex : public mutex {
public:
	thread_mutex()
	{
		::InitializeCriticalSection(&cs);
	}
	~thread_mutex()
	{
		::DeleteCriticalSection(&cs);
	}
	virtual void lock() {
		::EnterCriticalSection(&cs);
	}
	virtual void un_lock() {
		::LeaveCriticalSection(&cs);
	}
	virtual bool try_lock() {
		return true;
	}
	CRITICAL_SECTION cs;
};
#endif//thread_mutex
