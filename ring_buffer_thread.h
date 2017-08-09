#ifndef __ring_buffer_thread_h__
#define __ring_buffer_thread_h__


#include "ring_buffer.h"
#ifdef _WIN32
#include <process.h>
#endif


template<int PRIORITY=1>
class ring_buffer_thread
{
public:
	ring_buffer_thread() {
#ifdef _WIN32
		m_hEvent = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 1);
#else

#endif
	}
	~ring_buffer_thread() {

	}

protected:
private:
#ifdef 
	HANDLE m_hEvent;
#endif
};
#endif // __ring_buffer_thread_h__
