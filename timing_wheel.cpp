#include "timing_wheel.h"
#include "list_entry.h"
#include "util.h"

namespace shadow {
	//////////////////////////////////////////////////////////
#ifdef _DEBUG
	static int32_t count_ = 0;
	static int32_t max_timer_  = 0;
	static int64_t pc_ = 0;
#endif
	struct timer_base {
		uint32_t   tid;      // Timer ID
		uint32_t   delay;    // 设定的延时毫秒
		uint32_t   data;     // data
		int64_t    expires;  // 超时时间
		int64_t    start;    // 开始时间 (set_timer)的时间
		list_entry node;     // 链表节点
		bool          detach;// detach
		TimerState    state; // 状态
		TimerEvent    event; // 事件
#ifdef _DEBUG
		void print() {
			uint32_t interval;

			auto now = current_milliseconds();
			uint32_t cost = uint32_t(now - start);

			if (now >= expires) {
				interval = uint32_t(now - expires);
				printf("Timer[%u], cost[%u]ms, setting[%u]ms, +[%u]\n", tid, cost, delay, interval);

			}
			else {
				interval = uint32_t(expires - now);
				printf("Timer[%u], cost[%u]ms, setting[%u]ms, -[%u]\n", tid, cost, delay, interval);
			}

			count_++;
			pc_ += interval;
			if (count_ >= max_timer_) {
				double p = pc_ * 1.0f / count_;
				printf("共%d计时器，平均偏差%f毫秒\n", count_, p);
			}
		}
#endif
	};

	timer::timer()
		: detach_(false), tm_base_(nullptr) {

	}

	timer::timer(timer&& oth) {
		tm_base_ = oth.tm_base_;
		detach_ = oth.detach_;
		oth.tm_base_ = nullptr;
		oth.detach_ = true;
	}

	timer::timer(timer& oth) {
		tm_base_ = oth.tm_base_;
		detach_ = oth.detach_;
		oth.tm_base_ = nullptr;
		oth.detach_ = true;
	}

	timer& timer::operator=(timer&& oth) {
		if (this != &oth) {
			tm_base_ = oth.tm_base_;
			detach_ = oth.detach_;
			oth.tm_base_ = nullptr;
			oth.detach_ = true;
		}
		return *this;
	}

	timer timer::create(uint32_t tid, uint32_t delay, uint32_t data) {
		timer tm;
		tm.tm_base_ = new (std::nothrow) timer_base;
		if (tm.tm_base_) {
			tm.tm_base_->tid = tid;
			tm.tm_base_->delay = delay;
			tm.tm_base_->data = data;
			tm.tm_base_->expires = 0;
			tm.tm_base_->start = 0;
			tm.tm_base_->node.Flink = nullptr;
			tm.tm_base_->node.Blink = nullptr;
			tm.tm_base_->detach = false;
			tm.tm_base_->state = TimerState::TIMER_STATE_INIT;
			tm.tm_base_->event = TimerEvent::TIMER_EVENT_NULL;
		}
		return tm;
	}

	timer timer::create_null_timer() {
		timer tm; tm.detach_ = true;
		return tm;
	}

	timer& timer::operator = (timer& oth) {
		if (this != &oth) {
			tm_base_ = oth.tm_base_;
			detach_ = oth.detach_;
			oth.tm_base_ = nullptr;
			oth.detach_ = true;
		}
		return *this;
	}

	bool timer::detach() {
		if (is_valid()) {
			detach_ = true;
			tm_base_->detach = true;
			return true;
		}
		return false;
	}

	bool timer::is_detach() {
		return detach_;
	}

	bool timer::is_finish() {
		if (is_valid()) {
			return (tm_base_->state == TimerState::TIMER_STATE_FINISH);
		}
		return false;
	}

	uint32_t timer::get_delay() {
		uint32_t ret = 0;
		if (is_valid()) {
			switch (tm_base_->state) {
			case TimerState::TIMER_STATE_INIT:
			case TimerState::TIMER_STATE_QUEUE:
				ret = tm_base_->delay;
				break;
			case TimerState::TIMER_STATE_SETTING:
				ret = uint32_t(tm_base_->expires - current_milliseconds());
				break;
			case TimerState::TIMER_STATE_FINISH:
				ret = 0;
				break;
			}
		}
		return ret;
	}

	uint32_t timer::get_data() {
		if (is_valid()) {
			return tm_base_->data;
		}
		return 0;
	}

	uint32_t timer::get_timer_id() {
		if (is_valid()) {
			return tm_base_->tid;
		}
		return 0;
	}

	bool timer::is_valid() {
		return (tm_base_ && !detach_);
	}

	/////////////////////////////////////////////////////////////

#define GUARD(x) std::lock_guard<std::mutex> g(x)
#define CLOCK_TICK 5

//==========================================
//
//  from linux kernel
// 

#define TVR_BITS 8
#define TVN_BITS 6
#define TVR_SIZE (1 << TVR_BITS)
#define TVN_SIZE (1 << TVN_BITS)

	struct timer_vec {
		int32_t index;
		int16_t rsh;
		int16_t vsz;
		list_entry vec[TVN_SIZE];
	};

	struct timer_vec_root {
		int32_t index;
		int16_t rsh;
		int16_t vsz;
		list_entry vec[TVR_SIZE];
	};

	static struct timer_vec tv2;
	static struct timer_vec tv3;
	static struct timer_vec tv4;
	static struct timer_vec tv5;
	static struct timer_vec_root tv1;

//==========================================
	
/////////////////////////////////////////////////////////////

	timing_wheel::timing_wheel()
		: stopped_(true), init_(false), wait_clean_(false) {
		tvecs_[0] = (struct timer_vec*)&tv1;
		tvecs_[1] = &tv2;
		tvecs_[2] = &tv3;
		tvecs_[3] = &tv4;
		tvecs_[4] = &tv5;

		/* 
		// if we need multiple timers
		tvecs_[0] = (struct timer_vec*)new (std::nothrow) timer_vec_root;
		tvecs_[1] = new (std::nothrow) timer_vec;
		tvecs_[2] = new (std::nothrow) timer_vec;
		tvecs_[3] = new (std::nothrow) timer_vec;
		tvecs_[4] = new (std::nothrow) timer_vec;
		*/
	}

	timing_wheel* timing_wheel::instance() {
		static timing_wheel tw;
		return &tw;
	}

	timing_wheel::~timing_wheel()
	{
		stopped_ = true;
		init_ = false;

		while (!queue_.empty()) {
			auto t = queue_.front();
			queue_.pop_front();
			do_kill_timer(t);
		}

		// clean wheel list
		for (auto tv : tvecs_) {
			for (int16_t i = 0; i < tv->vsz; i++) {
				plist_entry list_head = &tv->vec[i];
				plist_entry entry = list_head->Flink;
				while (entry != list_head) {
					timer_base* item = CONTAINTING_RECORD(entry, timer_base, node);

					auto flink = entry->Flink;
					RemoveEntryList(entry);
					entry = flink;

					if (item->detach) {
						delete item;
					}
					else {
						item->state = TimerState::TIMER_STATE_FINISH;
					}
				}
			}
		}
	}

	void timing_wheel::thread_proc() {
		while (!stopped_) {
			process_event_and_timer();
		}
	}

	bool timing_wheel::init() {
		if (init_) return true;

		if (!init_win_socket()) {
			return false;
		}

		init_ = true;
		return true;
	}

	bool timing_wheel::start() {
		if (!stopped_) return false;
		if (wait_clean_) return false;

		current_msec_ = shadow::current_milliseconds();

		for (int32_t i = 0; i < 5; i++) {
			tvecs_[i]->index = 0;
		}
		for (int32_t i = 0; i < TVN_SIZE; i++) {
			InitializeListHead(&tvecs_[1]->vec[i]);
			InitializeListHead(&tvecs_[2]->vec[i]);
			InitializeListHead(&tvecs_[3]->vec[i]);
			InitializeListHead(&tvecs_[4]->vec[i]);
		}
		for (int32_t i = 0; i < TVR_SIZE; i++) {
			InitializeListHead(&tvecs_[0]->vec[i]);
		}

		tvecs_[0]->rsh = 0;// tv1.rsh = 0;
		tvecs_[1]->rsh = TVR_BITS;//tv2.rsh = 8;
		tvecs_[2]->rsh = TVR_BITS + TVN_BITS;//tv3.rsh = 8 + 6;
		tvecs_[3]->rsh = TVR_BITS + TVN_BITS + TVN_BITS;//tv4.rsh = 8 + 6 + 6;
		tvecs_[4]->rsh = TVR_BITS + TVN_BITS + TVN_BITS + TVN_BITS;//tv5.rsh = 8 + 6 + 6 + 6;
		tvecs_[0]->vsz = TVR_SIZE;
		tvecs_[1]->vsz = TVN_SIZE;
		tvecs_[2]->vsz = TVN_SIZE;
		tvecs_[3]->vsz = TVN_SIZE;
		tvecs_[4]->vsz = TVN_SIZE;

		stopped_ = false;
		activate();
		return true;
	}

	void timing_wheel::stop() {
		if (stopped_) return;
		if (wait_clean_) return;

		// kill all timers
		wait_clean_ = true;
		timer tm = timer::create(0, 0, 0);
		tm.tm_base_->event = TimerEvent::TIMER_EVENT_KILL_ALL;
		GUARD(mutex_);
		queue_.emplace_front(tm);
	}

	void timing_wheel::finish() {
		clean_win_socket();
		init_ = false;
	}

	bool timing_wheel::set_timer(timer& tm) {
		if (wait_clean_) return false;
		if (!(tm.tm_base_ && tm.tm_base_->state == TimerState::TIMER_STATE_INIT)) {
			return false;
		}

		GUARD(mutex_);
		tm.tm_base_->state = TimerState::TIMER_STATE_QUEUE;
		tm.tm_base_->event = TimerEvent::TIMER_EVENT_SET;
		tm.tm_base_->start = shadow::current_milliseconds();
		tm.tm_base_->expires = tm.tm_base_->start + tm.tm_base_->delay;

		if (tm.tm_base_->delay < TVR_SIZE) {
			queue_.emplace_front(tm);
		}
		else {
			queue_.emplace_back(tm);
		}
		return true;
	}

	bool timing_wheel::kill_timer(timer& tm) {
		if (wait_clean_) return false;
		if (tm.detach_ || !tm.tm_base_) {
			return false;
		}
		GUARD(mutex_);
		switch (tm.tm_base_->state) {
		case TimerState::TIMER_STATE_INIT:
		case TimerState::TIMER_STATE_FINISH:
			// not in queue OR already complete
			delete tm.tm_base_;
			break;
		case TimerState::TIMER_STATE_QUEUE:
			// still in queue
			// not in wheel list
			tm.tm_base_->event = TimerEvent::TIMER_EVENT_KILL;
			tm.tm_base_->detach = true;
			break;
		case TimerState::TIMER_STATE_SETTING:
			// already in wheel list
			// but not finish
			RemoveEntryList(&tm.tm_base_->node); // remove from list
			delete tm.tm_base_;
			break;
		}
		tm.tm_base_ = nullptr;
		tm.detach_ = true;
		return true;
	}

	bool timing_wheel::add_delay(timer& tm, uint32_t delay) {
		if (wait_clean_) return false;
		if (tm.detach_ || !tm.tm_base_) {
			return false;
		}
		GUARD(mutex_);
		if (tm.tm_base_->state != TimerState::TIMER_STATE_FINISH) {
			tm.tm_base_->delay += delay;
			tm.tm_base_->expires += delay;

			if (tm.tm_base_->state == TimerState::TIMER_STATE_SETTING) {
				RemoveEntryList(&tm.tm_base_->node); // remove from current list
				do_set_timer(tm); // insert again
			}
		}
		return true;
	}

	void timing_wheel::set_test_count(int32_t cc)
	{
#ifdef _DEBUG
		max_timer_ = cc;
		count_ = 0;
		pc_ = 0;
#endif
	}

	void timing_wheel::process_event_and_timer()
	{
		int64_t interval = 0;
		process_timer_event();

		do {
			interval = event_expire_timers();

			if (interval > 0) {
				shadow::sleep(interval > CLOCK_TICK ? CLOCK_TICK : interval);
				break;
			}

		} while (interval <= 0);
	}

	int64_t timing_wheel::event_expire_timers()
	{
		timer_vec_root* tvec = reinterpret_cast<timer_vec_root*>(tvecs_[0]);
		list_entry* list_head = &tvec->vec[tvec->index];
		list_entry* entry = list_head->Flink;

		int64_t ret = 1; // it's important, if list is empty, return 1
		current_msec_ = shadow::current_milliseconds();

		while (entry != list_head) {
			timer_base* item = CONTAINTING_RECORD(entry, timer_base, node);
			ret = item->expires - current_msec_;
			if (ret > 0) {
				return ret;
			}

			// TODO:
			// timer event or timer proc
#ifdef _DEBUG
			item->print();
#endif
			// move next, and remove entry
			auto flink = entry->Flink;
			RemoveEntryList(entry);
			entry = flink;

			if (item->detach) {
				delete item;
			}
			else {
				item->state = TimerState::TIMER_STATE_FINISH;
				item->detach = true;
			}
		}

		tvec->index++;

		// timer transfer
		if (tvec->index >= TVR_SIZE) {
			tvec->index = 0;
			if (timer_cascade(tvecs_[1]/*&tv2*/)) {
				if (timer_cascade(tvecs_[2]/*&tv3*/)) {
					if (timer_cascade(tvecs_[3]/*&tv4*/)) {
						if (timer_cascade(tvecs_[4]/*&tv5*/)) {

						}
					}
				}
			}
		}
		return ret;
	}

	bool timing_wheel::timer_cascade(struct timer_vec* tv)
	{
		do {

			list_entry* list_head = &tv->vec[tv->index];
			list_entry* entry = list_head->Flink;

			int64_t interval = 1;
			int32_t prev_limit = 1 << tv->rsh;

			current_msec_ = shadow::current_milliseconds();

			while (entry != list_head) {

				timer_base* item = CONTAINTING_RECORD(entry, timer_base, node);
				interval = item->expires - current_msec_;
				if (interval >= prev_limit) {
					return false;
				}

				auto flink = entry->Flink;
				RemoveEntryList(entry);
				entry = flink;

				do_set_timer(item);
			}

			tv->index++;

		} while (tv->index < TVN_SIZE);

		if (tv->index >= TVN_SIZE) {
			tv->index = 0;
			return true;
		}
		return false;
	}

	void timing_wheel::process_timer_event()
	{
		current_msec_ = shadow::current_milliseconds();

		timer tm = timer::create_null_timer();
		{
			GUARD(mutex_);
			if (queue_.empty()) {
				return;
			}
			tm = queue_.front();
			queue_.pop_front();
		}

		if (!tm.tm_base_) return; // impossible;

		switch (tm.tm_base_->event) {
		case TimerEvent::TIMER_EVENT_NULL:
			break;
		case TimerEvent::TIMER_EVENT_SET:
			do_set_timer(tm);
			break;
		case TimerEvent::TIMER_EVENT_KILL:
			do_kill_timer(tm);
			break;
		case TimerEvent::TIMER_EVENT_KILL_ALL:
			do_kill_all_timer(tm);
			break;
		}
	}

	void timing_wheel::do_set_timer(timer& tm)
	{
		uint32_t delta = 0;
		if (tm.tm_base_->expires > current_msec_) {
			delta = uint32_t((tm.tm_base_->expires - current_msec_) / CLOCK_TICK);
		}
		struct timer_vec* tvec = get_timer_vec(delta);

		auto slot = delta >> tvec->rsh;
		auto real = (slot + tvec->index) % tvec->vsz;

		list_entry* list_head = &tvec->vec[real];
		InsertTailList(list_head, &tm.tm_base_->node);
		tm.tm_base_->state = TimerState::TIMER_STATE_SETTING;
	}

	void timing_wheel::do_set_timer(struct timer_base* tb) {
		timer tm = timer::create_null_timer();
		tm.tm_base_ = tb;
		tm.detach_ = tb != nullptr ? tb->detach : true;
		do_set_timer(tm);
	}

	void timing_wheel::do_kill_timer(timer& tm) {
		if (tm.tm_base_ != nullptr) {
			delete tm.tm_base_;
		}
		tm.tm_base_ = nullptr;
		tm.detach_ = true;
	}

	void timing_wheel::do_kill_all_timer(timer& tm)
	{
		// clean tm
		do_kill_timer(tm);

		// clean queue
		while (!queue_.empty()) {
			auto t = queue_.front();
			queue_.pop_front();
			do_kill_timer(t);
		}

		// clean wheel list
		for (auto tv : tvecs_) {
			for (int16_t i = 0; i < tv->vsz; i++) {
				plist_entry list_head = &tv->vec[i];
				plist_entry entry = list_head->Flink;
				while (entry != list_head) {
					timer_base* item = CONTAINTING_RECORD(entry, timer_base, node);

					auto flink = entry->Flink;
					RemoveEntryList(entry);
					entry = flink;

					if (item->detach) {
						delete item;
					}
					else {
						item->state = TimerState::TIMER_STATE_FINISH;
					}
				}
			}
		}

		stopped_ = true;
		wait_clean_ = false;
	}

	struct timer_vec* timing_wheel::get_timer_vec(uint32_t interval)
	{
		struct timer_vec* tvec = nullptr;
		if (interval <= 0xff) {
			tvec = tvecs_[0];
		}
		else if (interval >= 0x100 && interval <= 0x3fff) {
			tvec = tvecs_[1];
		}
		else if (interval >= 0x4000 && interval <= 0xfffff) {
			tvec = tvecs_[2];
		}
		else if (interval >= 0x100000 && interval <= 0x3ffffff) {
			tvec = tvecs_[3];
		}
		else {
			// (interval >= 0x4000000 && interval <= 0xffffffff)
			tvec = tvecs_[4];
		}
		return tvec;
	}

}