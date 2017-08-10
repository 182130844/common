#ifndef __singleton_h__
#define __singleton_h__

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



#endif // __singleton_h__