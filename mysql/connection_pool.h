
#pragma once
#include <string>
#include <stdint.h>
#include "qlist.h"
#include "connection_base.h"

namespace shadow {

	class connection_pool
	{
	public:
		using pointer = void *;

		explicit connection_pool(const std::string& ip, uint32_t port, const std::string& user = "root", const std::string& pwd = "");

		std::string get_ip() const;
		uint32_t    get_port() const;
		std::string get_user() const;
		std::string get_pwd() const;

		// for hash map/set (unordered_map/unordered_set)
		std::size_t operator() () const;

		template<typename T>
		T* get_connection();

		void put_connection(pointer ptr);

		std::size_t get_hash_value() const;

		bool operator< (const connection_pool& oth) const;
	private:
		std::string    ip_;
		uint32_t       port_;
		std::string    user_;
		std::string    pwd_;
		qlist<pointer> pool_;
	};
}
