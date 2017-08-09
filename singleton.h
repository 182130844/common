#pragma once

template <class T>
class singleton {
public:
	static T* instance() { static T t; return &t; }
protected:
	singleton() = default;
private:
	singleton(const singleton&);
	singleton& operator= (const singleton&);
};