#pragma once

#include "mutex.h"
/*
-------------------------------------
          safe container
		  SHADOW 6/30 2017
		  shenzhen
-------------------------------------
*/
#include <queue>
using std::queue;

template <class T>
class squeue : public queue<T>
{
public:
	void push_ex(T& t) {
		auto_lock _al(&m_mutex);
		this->push(t);
	}

	void pop_ex(T& t) {
		auto_lock _al(&m_mutex);
		this->pop();
	}
private:
	thread_mutex m_mutex;
};

template <class T>
class svector
{
public:
	svector() {
		ptr = nullptr;
		max_size = len = 0;
	}
	~svector() {
		finish();
	}

	void init(int max_sz) {
		ptr = new (std::nothrow) T[max_sz];
		max_size = max_sz;
		len = 0;
	}

	void finish() {
		if (ptr) {
			delete[] ptr;
		}
		ptr = nullptr;
		max_size = len = 0;
	}
	bool add(T& t) {
		if (len >= max_size)
			return false;
		ptr[len++] = t;
		return true;
	}

	void clear() {
		len = 0;
	}
	void length() {
		return len;
	}
	T& operator[] (int i) {
		return ptr[i];
	}

	void resize(int ns) {
		len = ns;
	}

private:
	T* ptr;
	int max_size;
	int len;
};