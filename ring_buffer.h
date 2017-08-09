#ifndef __ring_buffer_h__
#define __ring_buffer_h__

#include "mutex.h"

class ring_buffer
{
public:
	ring_buffer();
	~ring_buffer();

	bool initialize(int init_size, int step_size, int max_size, int block_size);
	bool finish();
	void reset();

	bool push_data(void* head, int head_len, void* data, int data_len);
	bool pop_data(void* head, int& head_len, void* data, int& data_len);

	int  get_data_count() const;
	int  get_data_length() const;

private:

	thread_mutex m_mutex;

	char*        m_data_ptr;

	int          m_init_size;
	int          m_step_size;
	int          m_max_size;
	int          m_block_size;

	int          m_write_head;
	int          m_write_tail;
	int          m_read_head;

	int          m_data_count;
	int          m_data_length;
};
#endif // __ring_buffer_h__
