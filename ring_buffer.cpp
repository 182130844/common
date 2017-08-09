#include "ring_buffer.h"
#include <new>

struct ring_buffer_head {
	int head_len;
	int data_len;
};

const int RING_BUFFER_HEAD_LENGTH = sizeof(ring_buffer_head);

ring_buffer::ring_buffer() {
	
	m_data_ptr = nullptr;

	m_init_size = 0;
	m_step_size = 0;
	m_max_size = 0;
	m_block_size = 0;

	m_write_head = 0;
	m_write_tail = 0;
	m_read_head = 0;

	m_data_count = 0;
	m_data_length = 0;
}

ring_buffer::~ring_buffer() {
	if (m_data_ptr) {
		delete[] m_data_ptr;
		m_data_ptr = nullptr;
	}
}

bool ring_buffer::initialize(int init_size, int step_size, int max_size, int block_size) {
	if (m_data_ptr)
		return false;

	m_init_size = init_size;
	m_step_size = step_size;
	m_max_size = max_size;
	m_block_size = block_size;

	if (step_size < (block_size * 3))
		return false;

	m_data_ptr = new (std::nothrow) char[m_init_size];

	if (m_data_ptr) {
		m_write_head = 0;
		m_write_tail = 0;
		m_read_head = 0;

		m_data_count = 0;
		m_data_length = 0;
		return true;
	}
	return false;
}

bool ring_buffer::finish() {
	if (!m_data_ptr)
		return false;

	delete[] m_data_ptr;
	m_data_ptr = nullptr;

	m_init_size = 0;
	m_step_size = 0;
	m_max_size = 0;
	m_block_size = 0;

	m_write_head = 0;
	m_write_tail = 0;
	m_read_head = 0;

	m_data_count = 0;
	m_data_length = 0;
	return true;
}

void ring_buffer::reset() {
	auto_lock _al(&m_mutex);
	m_write_head = 0;
	m_write_tail = 0;
	m_read_head = 0;

	m_data_count = 0;
	m_data_length = 0;
}


bool ring_buffer::push_data(void* head, int head_len, void* data, int data_len) {
	auto_lock _al(&m_mutex);
	if (!m_data_ptr) return false;
	if (head_len > m_block_size || data_len > m_block_size) {
		return false;
	}

	if (!head || head_len <= 0) {
		head = nullptr;
		head_len = 0;
	}
	if (!data || data_len <= 0) {
		data = nullptr;
		data_len = 0;
	}

	if (!head && !data) {
		return false;
	}

	bool ret = true;
	int need_size = RING_BUFFER_HEAD_LENGTH + head_len + data_len;

	do {

		if (m_write_head == m_write_tail) {
			int tail_free_size = m_init_size - m_write_head;
			if (tail_free_size > need_size) { // enough free size of tail
				ring_buffer_head* ptr = reinterpret_cast<ring_buffer_head*>(&m_data_ptr[m_write_head]);
				ptr->head_len = head_len;
				ptr->data_len = data_len;
				if (head_len > 0) {
					memmove(&m_data_ptr[m_write_head + RING_BUFFER_HEAD_LENGTH], head, head_len);
				}
				if (data_len > 0) {
					memmove(&m_data_ptr[m_write_head + RING_BUFFER_HEAD_LENGTH + head_len], data, data_len);
				}

				m_write_head += need_size;
				m_write_tail = m_write_head;
			}
			else if (m_read_head > need_size) { // enough free size of head
				ring_buffer_head* ptr = reinterpret_cast<ring_buffer_head*>(&m_data_ptr[0]);
				ptr->head_len = head_len;
				ptr->data_len = data_len;

				if (head_len > 0) {
					memmove(&m_data_ptr[RING_BUFFER_HEAD_LENGTH], head, head_len);
				}
				if (data_len > 0) {
					memmove(&m_data_ptr[RING_BUFFER_HEAD_LENGTH + head_len], data, data_len);
				}

				m_write_head = need_size;
			}
			else {

				if (m_init_size + m_step_size > m_max_size) {
					ret = false;
					break;
				}

				m_init_size += m_step_size;
				char* ptr = new (std::nothrow) char[m_init_size];
				memmove(ptr, &m_data_ptr[m_read_head], m_write_head - m_read_head);
				m_write_head = m_write_head - m_read_head;
				m_write_tail = m_write_head;
				m_read_head = 0;
				
				delete[] m_data_ptr;
				m_data_ptr = ptr;

				ring_buffer_head* pptr = reinterpret_cast<ring_buffer_head*>(&m_data_ptr[m_write_head]);
				pptr->head_len = head_len;
				pptr->data_len = data_len;
				if (head_len > 0) {
					memmove(&m_data_ptr[m_write_head + RING_BUFFER_HEAD_LENGTH], head, head_len);
				}
				if (data_len > 0) {
					memmove(&m_data_ptr[m_write_head + RING_BUFFER_HEAD_LENGTH + head_len], data, data_len);
				}
				m_write_head += need_size;
				m_write_tail = m_write_head;
			}
		}
		else {

			int mid_free_size = m_read_head - m_write_head;
			if (mid_free_size > need_size) {
				ring_buffer_head* ptr = reinterpret_cast<ring_buffer_head*>(&m_data_ptr[m_write_head]);
				ptr->head_len = head_len;
				ptr->data_len = data_len;
				if (head_len > 0) {
					memmove(&m_data_ptr[m_write_head + RING_BUFFER_HEAD_LENGTH], head, head_len);
				}
				if (data_len > 0) {
					memmove(&m_data_ptr[m_write_head + RING_BUFFER_HEAD_LENGTH + head_len], data, data_len);
				}
				m_write_head += need_size;
			}
			else {
				if (m_init_size + m_step_size > m_max_size) {
					ret = false;
					break;
				}

				m_init_size += m_step_size;
				char* ptr = new (std::nothrow) char[m_init_size];
				int tail_data_size = m_write_tail - m_read_head;
				int head_data_size = m_write_head;
				memmove(ptr, &m_data_ptr[m_read_head], tail_data_size);
				memmove(&ptr[tail_data_size], m_data_ptr, head_data_size);

				delete[] m_data_ptr;
				m_data_ptr = ptr;

				m_write_head = tail_data_size + head_data_size;
				m_write_tail = m_write_head;
				m_read_head = 0;

				ring_buffer_head* pptr = reinterpret_cast<ring_buffer_head*>(&m_data_ptr[m_write_head]);
				pptr->head_len = head_len;
				pptr->data_len = data_len;

				if (head_len > 0) {
					memmove(&m_data_ptr[m_write_head + RING_BUFFER_HEAD_LENGTH], head, head_len);
				}
				if (data_len > 0) {
					memmove(&m_data_ptr[m_write_head + RING_BUFFER_HEAD_LENGTH + head_len], data, data_len);
				}
				m_write_head += need_size;
				m_write_tail = m_write_head;
			}
		}

	} while (false);

	if (ret) {
		m_data_count++;
		m_data_length += need_size;
	}
	return ret;
}

bool ring_buffer::pop_data(void* head, int& head_len, void* data, int& data_len) {
	auto_lock _al(&m_mutex);
	if (!m_data_ptr) return false;
	if (m_read_head == m_write_tail) {
		return false;
	}

	ring_buffer_head* ptr = reinterpret_cast<ring_buffer_head*>(&m_data_ptr[m_read_head]);
	if (ptr->head_len > head_len || ptr->data_len > data_len) {
		printf("ring_buffer->pop_data, small buffer\n");
		return false;
	}

	head_len = ptr->head_len;
	data_len = ptr->data_len;

	if (head_len > 0) {
		memmove(head, &m_data_ptr[m_read_head + RING_BUFFER_HEAD_LENGTH], head_len);
	}
	if (data_len > 0) {
		memmove(data, &m_data_ptr[m_read_head + RING_BUFFER_HEAD_LENGTH + head_len], data_len);
	}
	int read_size = RING_BUFFER_HEAD_LENGTH + head_len + data_len;

	m_read_head += read_size;
	m_data_length -= read_size;
	m_data_count--;

	if (m_read_head == m_write_tail && m_read_head == m_write_head) {
		m_read_head = 0;
		m_write_head = 0;
		m_write_tail = 0;
	}
	else if (m_read_head > m_write_head && m_read_head == m_write_tail) {
		m_read_head = 0;
		m_write_tail = m_write_head;
	}
	return true;
}

int ring_buffer::get_data_count() const {
	return m_data_count;
}

int ring_buffer::get_data_length() const {
	return m_data_length;
}
