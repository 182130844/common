#ifndef __ring_buffer_thread_h__
#define __ring_buffer_thread_h__


#include "ring_buffer.h"
#ifdef _WIN32
#include <process.h>
#else
#include <unistd.h>
#endif// _WIN32
#include <memory>

template<int MAX_PRIORITY_SIZE=1>
class ring_buffer_thread
{
public:
	ring_buffer_thread() {
#ifdef _WIN32
		m_hEvent = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 1);
#else
		pthread_attr_init(&m_attr);
		pthread_cond_init(&m_cond, 0);
		pthread_mutex_init(&m_mutex, 0);
#endif// _WIN32
		m_is_init = false;
		m_number_of_threads = 0;
		m_block_size = 0;
		m_is_run = false;
	}
	~ring_buffer_thread() {
#ifdef _WIN32
		if (m_hEvent != INVALID_HANDLE_VALUE) {
			::CloseHandle(m_hEvent);
		}
#else
		pthread_attr_destroy(&m_attr);
		pthread_cond_destroy(&m_cond);
		pthread_mutex_destroy(&m_mutex);
#endif// _WIN32
	}

	bool initialize(int number_of_threads, int init_size, int step_size, int max_size, int block_size) {
		if (m_is_init) return false;
		if (number_of_threads < 1) return false;

		m_number_of_threads = number_of_threads;
		m_block_size = block_size;

		for (int i = 0; i < MAX_PRIORITY_SIZE; i++) {
			if (m_data_buffer[i].initialize(init_size, step_size, max_size, block_size)) {
				m_is_init = true;
			}
		}
		return m_is_init;
	}

	bool finish() {
		if (!m_is_init || m_is_run) {
			return false;
		}

		for (int i = 0; i < MAX_PRIORITY_SIZE; i++) {
			m_data_buffer[i].finish();
		}
		m_is_init = false;
		return true;
	}

	bool start(int stack_size = 4 * 1024 * 1024) {
		if (stack_size < 0) stack_size = 0;
		if (!m_is_init || m_is_run) {
			return false;
		}
		m_is_run = true;
		for (int i = 0; i < m_number_of_threads; i++) {

#ifdef _WIN32
			unsigned int tid = 0;
			HANDLE hThread = (HANDLE)_beginthreadex(NULL, stack_size, _rbt_proc, this, 0, &tid);
			if (hThread == NULL) {
				return false;
			}
			CloseHandle(hThread);
			Sleep(100);

#else
			pthread_t tid;
			if (pthread_create(&tid, &m_attr, _rbt_proc, this) != 0) {
				return false;
			}
			usleep(100000);
#endif// _WIN32
		}
		return true;
	}

	bool stop() {
		if (!m_is_run) {
			return false;
		}

		m_is_run = false;
		for (int i = 0; i < m_number_of_threads; i++) {
#ifdef _WIN32
			PostQueuedCompletionStatus(m_hEvent, 0, NULL, NULL);
#else
			pthread_cond_signal(&m_cond);
#endif// _WIN32
		}
#ifdef _WIN32
		Sleep(1000);
#else
		sleep(1);
#endif// _WIN32
		return true;
	}

	bool put_data(void* head, int head_len, void* data, int data_len, int priority = 0) {
		if (!m_is_run) return false;
		if (priority < 0 || priority >= MAX_PRIORITY_SIZE) {
			priority = MAX_PRIORITY_SIZE - 1;
		}

		bool ret = m_data_buffer[priority].push_data(head, head_len, data, data_len);
		if (ret) {
#ifdef _WIN32
			PostQueuedCompletionStatus(m_hEvent, 0, NULL, NULL);
#else
			pthread_cond_signal(&m_cond);
#endif// _WIN32
		}
		return ret;
	}

	int get_data_count() const {
		int ret = 0;
		for (int i = 0; i < MAX_PRIORITY_SIZE; i++) {
			ret += m_data_buffer[i].get_data_count();
		}
		return ret;
	}

	virtual void on_data(void* head, int head_len, void* data, int data_len) = 0;

private:
#ifdef _WIN32
	static unsigned int* __stdcall _rbt_proc(void* ptr) {
#else
	static void*                   _rbt_proc(void* ptr) {
#endif// _WIN32
		ring_buffer_thread* pthis = reinterpret_cast<ring_buffer_thread*>(ptr);
		pthis->thread_proc();
		return 0;
	}

	void thread_proc() {
		std::unique_ptr<char[]> head(new (std::nothrow) char[m_block_size]);
		std::unique_ptr<char[]> data(new (std::nothrow) char[m_block_size]);

		while (m_is_run) {
#ifdef _WIN32
			DWORD dwSize = 0;
			ULONG ulKey = 0;
			LPOVERLAPPED lpOverlapped = nullptr;

			if (!GetQueuedCompletionStatus(m_hEvent, &dwSize, &ulKey, &lpOverlapped, INFINITE)) {
				break;
			}
#else
			pthread_mutex_lock(&m_mutex);
			pthread_cond_wait(&m_cond, &m_mutex);
			pthread_mutex_unlock(&m_mutex);
#endif // _WIN32

			if (!m_is_run) {
				break;
			}

			bool has_data = true;
			while (has_data) {
				int head_len = m_block_size;
				int data_len = m_block_size;
				has_data = false;

				for (int i = 0; i < MAX_PRIORITY_SIZE; i++) {
					bool ret = true;
					if (m_data_buffer[i].pop_data(head, head_len, data, data_len)) {
						has_data = true;
						try {
							on_data(head, head_len, data, data_len);
						}
						catch (...) {
							printf("ring_buffer_thread->on_data failed\n");
						}
						break;
					}
				} // end for
			}
		}
	}


private:

#ifdef _WIN32
	HANDLE m_hEvent;
#else
	pthread_attr_t m_attr;
	pthread_cond_t m_cond;
	pthread_mutex_t m_mutex;
#endif// _WIN32
	bool m_is_init;
	int  m_number_of_threads;
	int  m_block_size;
	bool m_is_run;

	ring_buffer m_data_buffer[MAX_PRIORITY_SIZE];
};
#endif // __ring_buffer_thread_h__
