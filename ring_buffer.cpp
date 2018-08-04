#include <string.h>
#include "ring_buffer.h"

namespace shadow {

	struct ring_buffer_head {
		int headsize;
		int datasize;
	};

#define GUARD(x) std::lock_guard<std::mutex> g(x)
#define BUFFER_HEAD_SIZE int(sizeof(ring_buffer_head))

	ring_buffer::ring_buffer()
	{
		m_data_ptr = 0;
		m_init_size = 0;
		m_step_size = 0;
		m_max_size = 0;
		m_max_buffer_size = 0;
		m_max_block_size = 0;

		m_write_tail = 0;
		m_write_head = 0;
		m_read_head = 0;

		m_data_count = 0;
		m_data_size = 0;

	}

	ring_buffer::~ring_buffer()
	{
	}

	bool ring_buffer::init(int init_size, int step_size, int max_size, int max_block_size)
	{
		if (m_data_ptr) {
			return false;
		}
		m_init_size = init_size;
		m_step_size = step_size;
		m_max_size = max_size;
		m_max_block_size = max_block_size;

		// the step_size must big than tripple of max_size
		if (step_size < (max_block_size * 3)) {
			return false;
		}

		m_data_ptr = new (std::nothrow) char[m_init_size];
		if (m_data_ptr == nullptr) return false;
		m_write_tail = 0;
		m_write_head = 0;
		m_read_head = 0;

		m_data_count = 0;
		m_data_size = 0;

		return true;
	}

	bool ring_buffer::fini()
	{
		if (!m_data_ptr)
			return false;
		
		delete m_data_ptr;
		m_data_ptr = 0;
		
		m_init_size = 0;
		m_step_size = 0;
		m_max_size = 0;
		m_max_buffer_size = 0;
		m_max_block_size = 0;

		m_write_tail = 0;
		m_write_head = 0;
		m_read_head = 0;

		m_data_count = 0;
		m_data_size = 0;

		return true;
	}

	int ring_buffer::get_data_count()
	{
		return m_data_count;
	}

	int ring_buffer::get_data_size()
	{
		return m_data_size;
	}

	bool ring_buffer::reset()
	{
		GUARD(mutex_);
		m_write_tail = 0;
		m_write_head = 0;
		m_read_head = 0;

		m_data_count = 0;
		m_data_size = 0;
		return true;
	}

	bool ring_buffer::putd(void* phead, int headlen, void* pdata, int datalen)
	{
		GUARD(mutex_);
		// has not initialize
		if (!m_data_ptr)
		{
			return false;
		}
		// illegal paramet
		if ((datalen > m_max_block_size) || (headlen > m_max_block_size))
		{
			return false;
		}
		if (!phead || headlen <= 0) { phead = 0; headlen = 0; }
		if (!pdata || datalen <= 0) { pdata = 0; datalen = 0; }

		if (headlen == 0 && datalen == 0) {
			return false;
		}

		int need_size = BUFFER_HEAD_SIZE + headlen + datalen;
		bool ret = true;
		do
		{
			if (m_write_head == m_write_tail)
			{
				int tail_free_size = m_init_size - m_write_head;
				// enough free size of tail
				if (tail_free_size > need_size)
				{
					ring_buffer_head* head = (ring_buffer_head*)&m_data_ptr[m_write_head];
					head->headsize = headlen;
					head->datasize = datalen;
					if (headlen > 0)
						memcpy(&m_data_ptr[m_write_head + BUFFER_HEAD_SIZE], phead, headlen);
					if (datalen > 0)
						memcpy(&m_data_ptr[m_write_head + BUFFER_HEAD_SIZE + headlen], pdata, datalen);
					m_write_head += need_size;
					m_write_tail = m_write_head;
				}
				else if (m_read_head > need_size)// enough free size of head
				{
					ring_buffer_head* head = (ring_buffer_head*)m_data_ptr;
					head->headsize = headlen;
					head->datasize = datalen;
					if (headlen > 0)
						memcpy(&m_data_ptr[BUFFER_HEAD_SIZE], phead, headlen);
					if (datalen > 0)
						memcpy(&m_data_ptr[BUFFER_HEAD_SIZE + headlen], pdata, datalen);
					m_write_head = need_size;
				}
				else
				{
					// if can expended buff
					if ((m_init_size + m_step_size) > m_max_size)
					{
						ret = false;
						break;
					}
					m_init_size += m_step_size;
					char* p_tmp = new (std::nothrow) char[m_init_size];
					if (p_tmp == nullptr) {
						ret = false;
						break;
					}
					memcpy(p_tmp, &m_data_ptr[m_read_head], (m_write_head - m_read_head));
					m_write_head = (m_write_head - m_read_head);
					m_write_tail = m_write_head;
					m_read_head = 0;

					delete[] m_data_ptr;
					m_data_ptr = p_tmp;
					ring_buffer_head* head = (ring_buffer_head*)&m_data_ptr[m_write_head];
					head->headsize = headlen;
					head->datasize = datalen;
					if (headlen > 0)
						memcpy(&m_data_ptr[m_write_head + BUFFER_HEAD_SIZE], phead, headlen);
					if (datalen > 0)
						memcpy(&m_data_ptr[m_write_head + BUFFER_HEAD_SIZE + headlen], pdata, datalen);
					m_write_head += need_size;
					m_write_tail = m_write_head;
				}
			}
			else
			{
				int midle_free_size = m_read_head - m_write_head;
				if (midle_free_size > need_size)
				{
					ring_buffer_head* head = (ring_buffer_head*)&m_data_ptr[m_write_head];
					head->headsize = headlen;
					head->datasize = datalen;
					if (headlen > 0)
						memcpy(&m_data_ptr[m_write_head + BUFFER_HEAD_SIZE], phead, headlen);
					if (datalen > 0)
						memcpy(&m_data_ptr[m_write_head + BUFFER_HEAD_SIZE + headlen], pdata, datalen);
					m_write_head += need_size;
				}
				else
				{
					if ((m_init_size + m_step_size) > m_max_size)
					{
						ret = false;
						break;
					}
					m_init_size += m_step_size;
					char* p_tmp = new (std::nothrow) char[m_init_size];
					if (p_tmp == nullptr) {
						ret = false;
						break;
					}
					int tail_data_size = m_write_tail - m_read_head;
					int head_data_size = m_write_head;
					memcpy(p_tmp, &m_data_ptr[m_read_head], tail_data_size);
					memcpy(&p_tmp[tail_data_size], m_data_ptr, head_data_size);
					delete[] m_data_ptr;
					m_data_ptr = p_tmp;

					m_write_head = head_data_size + tail_data_size;
					m_write_tail = m_write_head;
					m_read_head = 0;

					ring_buffer_head* head = (ring_buffer_head*)&m_data_ptr[m_write_head];
					head->headsize = headlen;
					head->datasize = datalen;
					if (headlen > 0)
						memcpy(&m_data_ptr[m_write_head + BUFFER_HEAD_SIZE], phead, headlen);
					if (datalen > 0)
						memcpy(&m_data_ptr[m_write_head + BUFFER_HEAD_SIZE + headlen], pdata, datalen);
					m_write_head += need_size;
					m_write_tail = m_write_head;
				}
			}
		} while (false);
		if (ret)
		{
			m_data_count++;
			m_data_size += need_size;
		}

		return ret;
	}

	bool ring_buffer::getd(void* phead, int& headlen, void* pdata, int& datalen)
	{
		GUARD(mutex_);
		if (!m_data_ptr) {
			return false;
		}
		if (!phead || !pdata) {
			return false;
		}
		if (m_read_head == m_write_tail)
			return false;

		ring_buffer_head* head = (ring_buffer_head*)&m_data_ptr[m_read_head];
		if (head->headsize > headlen || head->datasize > datalen)
			return false;
		headlen = head->headsize;
		datalen = head->datasize;
		if (headlen > 0)
			memcpy(phead, &m_data_ptr[m_read_head + BUFFER_HEAD_SIZE], headlen);
		if (datalen > 0)
			memcpy(pdata, &m_data_ptr[m_read_head + BUFFER_HEAD_SIZE + headlen], datalen);
		int read_size = headlen + datalen + BUFFER_HEAD_SIZE;

		m_read_head += read_size;
		m_data_count--;
		m_data_size -= read_size;

		if (m_write_tail == m_read_head && m_write_head == m_read_head)
		{
			m_read_head = 0;
			m_write_head = 0;
			m_write_tail = 0;
		}
		else if (m_write_tail > m_write_head && m_write_tail == m_read_head)
		{
			m_read_head = 0;
			m_write_tail = m_write_head;
		}

		return true;
	}
}
