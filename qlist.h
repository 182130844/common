

// FIFO queue

#pragma once
#include <list>

namespace shadow {

	template<typename T>
	class qlist : public std::list<T>
	{
	public:
		void push(T&& t) {
			this->emplace_back(t);
		}
		void push(const T& t) {
			this->emplace_back(t);
		}
		bool pop(T& t) {
			if (this->empty()) return false;
			t = this->front();
			this->pop_front();
			return true;
		}
	};
}

