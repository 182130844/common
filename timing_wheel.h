
#pragma once
#include <stdint.h>
#include <mutex>
#include <list>
#include "base_thread.h"

namespace shadow {


	enum class TimerState : unsigned char {
		TIMER_STATE_INIT,    // 初始化
		TIMER_STATE_QUEUE,   // 等待队列
		TIMER_STATE_SETTING, // 处于时间轮
		TIMER_STATE_FINISH   // 已经结束
	};

	enum class TimerEvent : unsigned char {
		TIMER_EVENT_NULL,     // 空
		TIMER_EVENT_SET,      // 设置定时器
		TIMER_EVENT_KILL,     // 结束定时器
		TIMER_EVENT_KILL_ALL  // 结束所有定时器
	};

	class timing_wheel;
	struct timer_base;

	class timer {
	public:
		~timer() = default;
		static timer create_null_timer();
		static timer create(uint32_t tid, uint32_t delay, uint32_t data);
		
		// copy constructor.
		timer(timer& oth);

		// copy assignment operator.
		timer& operator= (timer& oth);

		// move constructor.
		timer(timer&& oth);

		// move assignment operator.
		timer& operator= (timer&& oth);

		// 调用detach表示用户放弃对此定时器进行操作
		// 该定时器在超时结束时会自动释放
		// 若未调用detach，则在不需要使用时手动调
		// 用timing_wheel中的kill_timer函数清理
		bool detach();

		bool is_detach();
		bool is_finish();

		uint32_t get_delay();

		uint32_t get_data();

		uint32_t get_timer_id();

		bool is_valid();

	private:
		timer();
		bool detach_;
		struct timer_base* tm_base_;

		friend class timing_wheel;
	};


	struct timer_vec;

	class timing_wheel : public base_thread {
	public:
		~timing_wheel();
		virtual void thread_proc() override;

		static timing_wheel* instance();

		bool init();
		bool start();
		void stop();
		void finish();

		bool set_timer(timer& tm);
		bool kill_timer(timer& tm);
		bool add_delay(timer& tm, uint32_t delay);
		void set_test_count(int32_t cc);

	private:
		timing_wheel();
		timing_wheel(const timing_wheel& oth) = delete;
		timing_wheel& operator = (const timing_wheel& oth) = delete;

		void process_event_and_timer();
		int64_t event_expire_timers();
		void process_timer_event();
		bool timer_cascade(struct timer_vec* tv);

		// internal timer operate
		void do_set_timer(timer& tm);
		void do_set_timer(struct timer_base* tb);

		void do_kill_timer(timer& tm);
		void do_kill_all_timer(timer& tm);

		struct timer_vec* get_timer_vec(uint32_t interval);
	private:

		bool stopped_;
		bool init_;
		bool wait_clean_;
		std::mutex mutex_;
		std::list<timer> queue_;

		int64_t current_msec_;
		struct timer_vec* tvecs_[5];

	};

#define twmgr timing_wheel::instance()
}