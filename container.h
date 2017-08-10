#ifndef __container_h__
#define __container_h__

#include "mutex.h"
#include "byte_stream.h"

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
	void push(T& t) {
		auto_lock _al(&m_mutex);
		__super::push(t);
	}

	void pop(T& t) {
		auto_lock _al(&m_mutex);
		if (empty()) return;
		t = this->front();
		__super::pop();
	}
private:
	thread_mutex m_mutex;
};

template<class T, int MAX_SIZE>
class mvector
{
public:
	mvector() :m_size(0) {}
	~mvector() = default;
	T& operator[] (int index) { return m_data[index]; }
	void clear() { m_size = 0; }
	int size() const { return m_size; }
	void size(int ns) { m_size = ns;
		if (m_size < 0) m_size = 0;
		if (m_size > MAX_SIZE) m_size = MAX_SIZE;
	}
	bool inc_size() { // return true when reach end
		m_size++;
		if (m_size > MAX_SIZE) m_size = MAX_SIZE;
		return (m_size == MAX_SIZE);
	}
	bool add(T&& t) {
		if (m_size >= MAX_SIZE) return false;
		m_data[m_size] = t;
		inc_size();
		return true;
	}

	bool serialize_in(byte_stream& bs) {
		short len = 0;
		bs.read(len);

		if (len <0 || len>MAX_SIZE)
			return false;

		for (auto& it : m_data) {
			if (!it.serialize_in(bs)) { // read data from bs
				return false;
			}
		}
		m_size = len;
		return true;
	}

	bool serialize_out(byte_stream& bs) {
		short len = (short)m_size;
		if (!bs.write(len))
			return false;

		for (auto& it : m_data) {
			if (!it.serialize_out(bs)) { // write into bs
				return false;
			}
		}
		return true;
	}

private:
	int m_size;
	T   m_data[MAX_SIZE];
};

template<int MAX_SIZE, class T>
class ustring
{
public:
	ustring() {
		m_data[0] = 0;
		m_length = 0;
	}
	~ustring() = default;

	const ustring& operator = (const char* ptr) { set(ptr); return *this; }
	const ustring& operator = (const ustring& str) {
		m_length = (str.m_length < MAX_SIZE) ? str.m_length : (MAX_SIZE - 1);
		memmove(m_data, str.m_data, m_length);
		m_data[m_length] = 0;
		return *this;
	}
	ustring& operator += (char* ptr) { return add(ptr); }
	ustring& operator += (ustring& str) { return add(str.m_data); }

	int length() const { return m_length; }
	void clear() {
		m_length = 0;
		m_data[0] = 0;
	}
	void set(const char* ptr) {
		if (!ptr) {
			clear();
			return;
		}

		m_length = strlen(ptr);
		if (m_length >= MAX_SIZE)
			m_length = MAX_SIZE - 1;

		memmove(m_data, ptr, m_length);
		m_data[m_length] = 0;
	}
	ustring& add(char* ptr) {
		if (ptr) {
			int len = strlen(ptr);
			if (len + m_length < MAX_SIZE) {
				memmove(&m_data[m_length], ptr, len);
				m_length += len;
				m_data[m_length] = 0;
			}
		}
		return *this;
	}

	bool serialize_in(byte_stream& bs) {
		T len = 0;
		if (!bs.read(len) || len > sizeof(m_data) - 2)
			return false;
		if (!bs.read_value(m_data, len))
			return false;

		m_length = len;
		m_data[m_length] = 0;
		return true;
	}
	bool serialize_out(byte_stream& bs) {
		T len = m_length;
		if (len > sizeof(m_data) - 2)
			len = sizeof(m_data) - 2;

		if (!bs.write(len)) return false;
		if (!bs.write_value(m_data, len)) return false;
		return true;
	}

	void set_json(const char* s) {
		if (!s) {
			clear();
			return;
		}
		int len = strlen(s);
		m_length = 0;
		for (int i = 0; i < len; ) {
			if (s[i] == '#') {
				if (((i + 1) < len) && s[i + 1] == '#') {
					m_data[m_length++] = '#';
					i += 2;
				}
				else if (((i + 3) < len) && s[i + 1] == '3' && s[i + 2] == '4' && s[i + 3] == ';') {
					m_data[m_length++] = '"';
					i += 4;
				}
				else if (((i + 3) < len) && s[i + 1] == '1' && s[i + 2] == '0' && s[i + 3] == ';') {
					m_data[m_length++] = '\n';
					i += 4;
				}
				else {
					m_data[m_length++] = s[i];
					i++;
				}
			}
			else {
				m_data[m_length++] = s[i];
				i++;
			}
			if (m_length >= MAX_SIZE)
				break;
		}
		m_data[m_length] = 0;
	}
	int  print_json(char* buf, int buf_len) {
		int size = 0;
		for (int i = 0; i < m_length; i++) {
			if (m_data[i] == '"') { buf[size++] = '#'; buf[size++] = '3'; buf[size++] = '4'; buf[size++] = ';'; }
			else if (m_data[i] == '\n') { buf[size++] = '#'; buf[size++] = '1'; buf[size++] = '0'; buf[size++] = ';'; }
			else if (m_data[i] == '#') { buf[size++] = '#'; buf[size++] = '#'; }
			else buf[size++] = m_data[i];
			if (size >= buf_len) break;
		}
		buf[size] = 0;
		return size;
	}
private:
	int  m_length;
	char m_data[MAX_SIZE + 2];
};
#endif // __container_h__