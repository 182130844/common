#include "timer.h"

struct timer_key {
	union {
		struct {
			ushort id1;
			ushort id2;
			ushort id3;
			ushort tid;
		};

		ulonglong  key;
	};
};


#define TYPE_SET_TIMER  1
#define TYPE_KILL_TIMER 2
#define TYPE_ADD_TIME   3

timer::timer()
	: base_thread("timer")
	, m_is_run(false)
	, m_i_service(nullptr) {


}

bool timer::initialize(interface_timer_service* s, long time_unit) {
	if (!s) return false;
	m_i_service = s;
	m_unit_of_time = time_unit;
	return true;
}

bool timer::start() {
	if (!m_i_service || m_is_run)
		return false;

	m_is_run = true;
	activate();
	return true;
}
bool timer::stop() {
	if (!m_is_run) return false;
	m_is_run = false;
#ifdef _WIN32
	Sleep(1500);
#else
	usleep(1500 * 1000);
#endif
	return true;
}
int timer::on_thread_proc() {
	while (m_is_run) {
#ifdef _WIN32
		Sleep(m_unit_of_time);
#else
		usleep(m_unit_of_time * 1000);
#endif
		check_setting_queue();

		if (!m_is_run)
			break;

		check_timer();
	}
	m_timer_map.clear();
	return 0;
}

void timer::set_timer(ushort id1, ushort id2, ushort id3, ushort tid, int delay, ulong data) {
	if (!m_is_run)
		return;

	timer_key tk;
	tk.id1 = id1;
	tk.id2 = id2;
	tk.id3 = id3;
	tk.tid = tid;

	timer_item_setting s;
	s.type = TYPE_SET_TIMER;
	s.key = tk.key;
	s.delay = delay;
	s.data = data;

	m_settings.push(s);
}

void timer::kill_timer(ushort id1, ushort id2, ushort id3, ushort tid) {
	timer_key tk;
	tk.id1 = id1;
	tk.id2 = id2;
	tk.id3 = id3;
	tk.tid = tid;

	timer_item_setting s;
	s.type = TYPE_KILL_TIMER;
	s.key = tk.key;
	s.delay = 0;
	s.data = 0;

	m_settings.push(s);
}

bool timer::add_delay(ushort id1, ushort id2, ushort id3, ushort tid, int delay) {
	if (!m_is_run)
		return false;

	timer_key tk;
	tk.id1 = id1;
	tk.id2 = id2;
	tk.id3 = id3;
	tk.tid = tid;

	timer_item_setting s;
	s.type = TYPE_ADD_TIME;
	s.key = tk.key;
	s.delay = delay;
	s.data = 0;

	auto i = m_timer_map.find(s.key);
	if (i == m_timer_map.end())
		return false;

	m_settings.push(s);
	return true;
}

int timer::get_delay(ushort id1, ushort id2, ushort id3, ushort tid) {
	if (!m_is_run)
		return 0;

	timer_key tk;
	tk.id1 = id1;
	tk.id2 = id2;
	tk.id3 = id3;
	tk.tid = tid;

	auto_lock _al(&m_mutex);
	timer_iterator i = m_timer_map.find(tk.key);
	if (i == m_timer_map.end())
		return 0;

	return i->second.count;
}

void timer::check_timer() {
	timer_iterator it;
	for (it = m_timer_map.begin(); it != m_timer_map.end();) {
		it->second.count--;
		if (it->second.count <= 0) {
			timer_key tk;
			tk.key = it->first;
			m_i_service->on_timer(tk.id1, tk.id2, tk.id3, tk.tid, it->second.data);
			auto_lock _al(&m_mutex);
			it = m_timer_map.erase(it);
		}
		else {
			it++;
		}
	}
}

void timer::check_setting_queue() {
	auto_lock _al(&m_mutex);
	timer_item_setting s;
	timer_iterator it;

	while (m_settings.pop(s)) {
		switch (s.type) {
		case TYPE_SET_TIMER:
		{
			timer_item item;
			item.count = s.delay;
			item.delay = s.delay;
			item.data = s.data;
			m_timer_map[s.key] = item;
		}
		break;
		case TYPE_KILL_TIMER:
		{
			it = m_timer_map.find(s.key);
			if (it != m_timer_map.end())
				m_timer_map.erase(it);
		}
		break;
		case TYPE_ADD_TIME:
		{
			it = m_timer_map.find(s.key);
			if (it != m_timer_map.end())
				it->second.count += s.delay;
		}
		break;
		}
	}
}