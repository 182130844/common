#ifndef __mutex_h__
#define __mutex_h__

#ifdef _WIN32
#include <windows.h>
#else
#include <pthread.h>
#include <unistd.h>
#endif
#include <stdio.h>

#include <list>
using std::list;


class mutex
{
public:
	virtual void lock() = 0;
	virtual void unlock() = 0;
	virtual bool try_lock() = 0;
};

class auto_lock
{
public:
	auto_lock(mutex* ptr) :_lock(ptr) {
		if (_lock) {
			_lock->lock();
		}
	}
	~auto_lock() {
		if (_lock) {
			_lock->unlock();
		}
	}
private:
	mutex* volatile _lock;
};

#ifdef _WIN32
class thread_mutex : public mutex
{
public:
	thread_mutex() {
		::InitializeCriticalSection(&_cs);
	}
	~thread_mutex() {
		::DeleteCriticalSection(&_cs);
	}
	virtual void lock() override {
		::EnterCriticalSection(&_cs);
	}
	virtual void unlock() override {
		::LeaveCriticalSection(&_cs);
	}
	virtual bool try_lock() override {
		return true;
	}
private:
	CRITICAL_SECTION _cs;
};

#else

class thread_mutex : public mutex
{
public:
	thread_mutex() {
		pthread_mutexattr_settype(&_attr, PTHREAD_MUTEX_RECURSIVE);
		pthread_mutex_init(&_mutex);
	}
	~thread_mutex() {
		pthread_mutex_destroy(&_mutex);
	}
	virtual void lock() override {
		pthread_mutex_lock(&_mutex);
	}
	virtual void unlock() override {
		pthread_mutex_unlock(&_mutex);
	}
	virtual bool try_lock() override {
		return (pthread_mutex_trylock(&_mutex) == 0);
	}
private:
	pthread_mutexattr_t   _attr;
	pthread_mutex_t      _mutex;
};
#endif


#endif // __mutex_h__
