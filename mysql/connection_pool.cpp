
#include "connection_pool.h"
#include "hash_value.h"

namespace shadow {

	connection_pool::connection_pool(const std::string& ip, uint32_t port, const std::string& user, const std::string& pwd)
		: ip_(ip), port_(port), user_(user), pwd_(pwd) {
	};

	std::string connection_pool::get_ip() const {
		return ip_;
	}

	uint32_t connection_pool::get_port() const
	{
		return port_;
	}

	std::string connection_pool::get_user() const
	{
		return user_;
	}

	std::string connection_pool::get_pwd() const
	{
		return pwd_;
	}

	std::size_t connection_pool::operator()() const {
		return get_hash_value();
	}

	template<typename T>
	T* connection_pool::get_connection() {
		connection_pool::pointer ptr = nullptr;
		pool_.pop(ptr);
		if (!ptr) {
			connection_base* p = new (std::nothrow) T;
			p->init();
			if (!p->open(ip_.c_str(), user_.c_str(), pwd_.c_str(), "", port_)) {
				delete p;
			}
			else {
				ptr = reinterpret_cast<pointer>(p);
			}
		}
		return reinterpret_cast<T*>(ptr);
	}

	void connection_pool::put_connection(pointer ptr)
	{
		if (!ptr) return;
		pool_.push(ptr);
	}

	std::size_t connection_pool::get_hash_value() const
	{
		return hash_value(ip_, port_, std::string(user_));
	}

	bool connection_pool::operator<(const connection_pool& oth) const
	{
		auto hv = get_hash_value();
		return hv < oth.get_hash_value();
	}

}
