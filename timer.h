#ifndef __timer_h__
#define __timer_h__

#include "singleton.h"
#include "base_thread.h"
#include "container.h"
#include <map>
using std::map;

using ushort = unsigned short;
using ulong = unsigned long;
using ulonglong = unsigned long long;

class interface_timer_service
{
public:
	virtual void on_timer(ushort tid1, ushort tid2, ushort tid3, ushort tid, int data) = 0;
};

struct timer_item {
	int delay;
	int count;
	ulong data;
};

using timer_map = map<ulonglong, timer_item>;
using timer_iterator = timer_map::iterator;

struct timer_item_setting {
	int type;
	ulonglong key;
	int delay;
	ulong data;
};

using timer_setting_queue = squeue<timer_item_setting>;

class timer: public base_thread, public singleton<timer>
{
public:
	timer();
	~timer() = default;

	virtual int on_thread_proc() override;
	bool initialize(interface_timer_service* s, long time_unit = 1000 /* ms, defalut 1 second */);
	bool start();
	bool stop();

	//
	// delay, the number of times to be executed,
	// so real delay time is (delay * time_unit).
	//
	void set_timer(ushort id1, ushort id2, ushort id3, ushort tid, int delay, ulong data);
	void kill_timer(ushort id1, ushort id2, ushort id3, ushort tid);
	bool add_delay(ushort id1, ushort id2, ushort id3, ushort tid, int delay);
	int  get_delay(ushort id1, ushort id2, ushort id3, ushort tid);

private:

	void check_timer();
	void check_setting_queue();

	bool m_is_run;
	long m_unit_of_time;

	thread_mutex m_mutex;
	timer_setting_queue m_settings;

	timer_map m_timer_map;
	interface_timer_service* m_i_service;
};

#define timerInst  timer::instance()
#define SET_TIMER  timerInst->set_timer
#define KILL_TIMER timerInst->kill_timer
#define ADD_DELAY  timerInst->add_delay
#define GET_DELAY  timerInst->get_delay

#endif // __timer_h__
