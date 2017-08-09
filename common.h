#ifndef __common_h__
#define __common_h__

#include <list>
#include <map>
#include <vector>
#include <string>
#include <memory>

using std::string;
using std::vector;
using std::list;
using std::map;
using std::shared_ptr;
using std::unique_ptr;

template <class A, class B>
shared_ptr<A> As(const B& ptr) {
	return std::dynamic_pointer_cast<A>(ptr);
}

template <class A, class B>
bool Is(const B& ptr) {
	return bool(As<A, B>(ptr));
}

template <class T, class... Args>
inline shared_ptr<T> New(Args&&... args) {
	return std::make_shared<T>(std::forward<Args>(args)...);
}

#endif // __common_h__
