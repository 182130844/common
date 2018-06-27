

#pragma once

template <typename T>
class singleton {
public:
	static T* instance() { static T t; return &t; }
	~singleton() = default;
protected:
	singleton() = default;
private:
	singleton(const singleton&) = delete;
	singleton& operator= (const singleton&) = delete;
};
