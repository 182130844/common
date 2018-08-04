
#pragma once
#include <memory>

template <class A, class B>
std::shared_ptr<A> As(const B& ptr) {
	return std::dynamic_pointer_cast<A>(ptr);
}

template <class A, class B>
bool Is(const B& ptr) {
	return bool(As<A, B>(ptr));
}

template <class T, class... Args>
inline std::shared_ptr<T> New(Args&&... args) {
	return std::make_shared<T>(std::forward<Args>(args)...);
}

// 10进制转其他进制(10以下)，默认10转8
template<typename T, int radix = 8>
T Conversion(T value) {
	T ret = value % radix;
	int i = 0;
	do {
		value = value / radix;
		if (value != 0) {
			i++;

			T  base = (value % radix);
			for (int j = 0; j < i; j++) {
				base *= 10;
			}
			ret += base;
		}

	} while (value >= radix);

	return ret;
}
