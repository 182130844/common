
#pragma once
#include "ring_buffer.h"
#ifdef _WIN32
#include <Windows.h>
#include <process.h>
#endif

namespace shadow {

	template<int RBT_MAX_PRIORITY = 1>
	class ring_buffer_thread {
	public:
		ring_buffer_thread()
		{
#ifdef _WIN32
			m_hDataEvent = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 1);
#else
			pthread_attr_init(&m_thd_attr);
			pthread_cond_init(&m_thd_cond, 0);
			pthread_mutex_init(&m_thd_mutex, 0);
#endif
			m_init = false;
			m_threads = 0;
			m_max_block_size = 0;
			m_running = false;
		}
		~ring_buffer_thread()
		{
#ifdef _WIN32
			if (m_hDataEvent != INVALID_HANDLE_VALUE)
			{
				::CloseHandle(m_hDataEvent);
				m_hDataEvent = INVALID_HANDLE_VALUE;
			}
#else
			pthread_attr_destroy(&m_thd_attr);
			pthread_cond_destroy(&m_thd_cond);
			pthread_mutex_destroy(&m_thd_mutex);
#endif
		}
		bool is_running() const { return m_running; };

		/*
		 * 初始化环形缓存与工作线程 
		 * @param threads: 线程数
		 * @param init_size: 每个环形缓存初始的长度
		 * @param step_size: 环形缓存每次扩展时增加的长度
		 * @param max_size:  环形缓存允许的最大长度
		 * @param max_block_size: 每次提交的数据块最大长度
		 */
		bool init(int threads, int init_size, int step_size, int max_size, int max_block_size) {
			if (m_init) {
				return false;
			}
			if (threads <= 0 || max_block_size <= 0) {
				return false;
			}

			m_threads = threads;
			m_max_block_size = max_block_size;

			for (int i = 0; i < RBT_MAX_PRIORITY; i++)
			{
				if (!m_data_buff[i].init(init_size, step_size, max_size, max_block_size)) {
					return false;
				}
			}

			m_init = true;
			return true;
		}

		bool fini()
		{
			if (!m_init || m_running)
			{
				return false;
			}
			for (int i = 0; i < RBT_MAX_PRIORITY; i++)
			{
				m_data_buff[i].fini();
			}
			m_init = false;
			return true;
		}

		bool start(int stackSize = 1024 * 1024 * 4)
		{
			if (stackSize < 0)stackSize = 0;
			if (!m_init || m_running)
			{
				return false;
			}
	
			m_running = true;

			for (int i = 0; i < m_threads; i++) {
#ifdef _WIN32
				unsigned int dwThreadID = 0;
				HANDLE hThread = (HANDLE)_beginthreadex(NULL, stackSize, threadproc_, this, 0, &dwThreadID);
				if (hThread == NULL || hThread == INVALID_HANDLE_VALUE) return false;
				Sleep(100);
#else
				pthread_t tid;
				if (pthread_create(&tid, &m_thd_attr, threadproc_, this) != 0)
					return false;
#endif
			}
			return true;
		}

		bool stop()
		{
			if (!m_running) {
				return false;
			}
			m_running = false;

			for (int i = 0; i < m_threads; i++) {
#ifdef _WIN32
				::PostQueuedCompletionStatus(m_hDataEvent, 0, 0, 0);
#else
				pthread_cond_signal(&m_thd_cond);
#endif
			}

#ifdef _WIN32
			Sleep(1000);
#else
			sleep(1);
#endif
			return true;
		}

		bool put_data(void* phead, int headlen, void* pdata, int datalen, int pri = 0)
		{
			if (!m_running) {
				return false;
			}

			if (pri < 0 || pri >= RBT_MAX_PRIORITY) {
				pri = RBT_MAX_PRIORITY - 1;
			}

			if (m_data_buff[pri].putd(phead, headlen, pdata, datalen)) {
#ifdef _WIN32
				::PostQueuedCompletionStatus(m_hDataEvent, 0, 0, 0);
#else
				pthread_cond_signal(&m_thd_cond);
#endif
				return true;
			}
			return false;
		}

		bool put_data(void* phead, int headlen, int pri = 0) {
			return put_data(phead, headlen, 0, 0, pri);
		}

		int get_data_count() {
			int sum = 0;
			for (int i = 0; i < RBT_MAX_PRIORITY; i++) {
				sum += m_data_buff[i].get_data_count();
			}
			return sum;
		}

		virtual void on_data(void* phead, int headlen, void* pdata, int datalen) = 0;

	private:
#ifdef _WIN32
		static unsigned int __stdcall threadproc_(void * lparam)
#else
		static void* threadproc_(void* lparam)
#endif
		{
			ring_buffer_thread* pthis = (ring_buffer_thread*)lparam;
			pthis->process();
			return 0;
		}

		int process()
		{
			char* phead = new (std::nothrow) char[m_max_block_size];
			char* pdata = new (std::nothrow) char[m_max_block_size];
			while (m_running) {
#ifdef _WIN32
				ULONG completionKey = 0;
				LPOVERLAPPED overLapped;
				DWORD dwNumberOfBytesTransferred = 0;
				if (!::GetQueuedCompletionStatus(m_hDataEvent, &dwNumberOfBytesTransferred, &completionKey, &overLapped, INFINITE)) {
					Sleep(10);
					continue;
				}
#else
				pthread_mutex_lock(&m_thd_mutex);
				pthread_cond_wait(&m_thd_cond, &m_thd_mutex);
				pthread_mutex_unlock(&m_thd_mutex);
#endif
				if (!m_running) {
					break;
				}
				bool find_data = true;
				while (find_data) {

					int headlen = m_max_block_size;
					int datalen = m_max_block_size;
					
					find_data = false;

					for (int i = 0; i < RBT_MAX_PRIORITY; i++) {

						if (m_data_buff[i].getd(phead, headlen, pdata, datalen)) {
						
							find_data = true;
							try {
								on_data(phead, headlen, pdata, datalen);
							}
							catch (...) {

							}
							break;
						}
					}
				}
			}
			m_running = false;
			delete[] phead;
			delete[] pdata;
			return 0;
		}


	private:
		bool            m_init;
		volatile bool   m_running;
		int             m_threads;
		int             m_max_block_size;
#ifdef _WIN32
		HANDLE			m_hDataEvent;
#else
		pthread_attr_t  m_thd_attr;
		pthread_cond_t  m_thd_cond;
		pthread_mutex_t m_thd_mutex;
#endif
		ring_buffer    m_data_buff[RBT_MAX_PRIORITY];
	};
}


