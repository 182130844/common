
#pragma once
#include <mutex>

namespace shadow {

	class ring_buffer
	{
	public:
		ring_buffer();
		~ring_buffer();
		bool init(int init_size, int step_size, int max_size, int max_block_size);
		bool fini();
		bool reset();
		bool putd(void* phead, int headlen, void* pdata, int datalen);
		bool getd(void* phead, int& headlen, void* pdata, int& datalen);
		int  get_data_count();
		int  get_data_size();

	private:
		std::mutex mutex_;

		char* m_data_ptr;
		int m_init_size;
		int m_step_size;
		int m_max_size;
		int m_max_buffer_size;
		int m_max_block_size;

		int m_write_tail;
		int m_write_head;
		int m_read_head;

		int m_data_count;
		int m_data_size;
	};

}