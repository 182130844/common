#ifndef __byte_stream_h__
#define __byte_stream_h__

#include <string.h>

class byte_stream
{
public:
	byte_stream(char* ptr, int len)
		:m_ptr(ptr), m_length(len), m_current(0) {}
	~byte_stream() = default;

	bool write_value(char* ptr, int len) {
		if (m_length < m_current + len) {
			return false;
		}

		memmove(&m_ptr[m_current], ptr, len);
		m_current += len;
		return true;
	}

	bool read_value(char* ptr, int len) {
		if (m_length < m_current + len) {
			return false;
		}

		memmove(ptr, &m_ptr[m_current], len);
		m_current += len;
		return true;
	}

	template<class T>
	bool write(T& t) {
		return write_value(reinterpret_cast<char*>(&t), sizeof(T));
	}

	template<class T>
	bool read(T& t) {
		return read_value(reinterpret_cast<char*>(&t), sizeof(T));
	}

private:
	int   m_length;
	int   m_current;
	char* m_ptr;
};


#endif // __byte_stream_h__