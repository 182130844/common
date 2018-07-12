//=====================================================================
// 
// shadow_yuan@qq.com
// shenzhen 6/27 2018
//
//=====================================================================

#pragma once
#include <functional>
#include <vector>
#include <thread>
#include <queue>
#include <mutex>
#include <atomic>
#include <future>
#include <condition_variable>

namespace shadow {

class thread_pool
{
public:
	using Task = std::function<void()>;

	thread_pool(size_t num) {
		for (size_t i = 0; i < num; i++) {
			thread_pool_.emplace_back(&thread_pool::schedule, this);
		}
	}
	~thread_pool() {
		stopped_.store(true);
		cond_var_.notify_all();
		for (auto& thread : thread_pool_) {
			if (thread.joinable()) {
				thread.join();
			}
		}
	}

	template<typename F, typename... Args>
	auto commit(F&& f, Args&&... args) ->std::future< decltype(f(args...)) > {
		if (stopped_.load()) {
			throw std::runtime_error("[thread pool] commit on thread pool was stopped.");
		}
		using retType = decltype(f(args...));
		auto task = std::make_shared<std::packaged_task<retType()>> (
			std::bind(std::forward<F>(f), std::forward<Args>(args)...)
			);

		std::future<retType> future = task->get_future();
		{
			std::lock_guard<std::mutex> lock(mutex_);
			task_queue_.emplace([task]() {
				(*task)();
			});
		}
		cond_var_.notify_one();
		return future;
	}

	void shutdown() {
		stopped_.store(true);
		cond_var_.notify_all();
	}

	bool is_stopped() {
		return stopped_.load();
	}

private:
	Task get_one_task() {
		std::unique_lock<std::mutex> lock(mutex_);
		cond_var_.wait(lock, [this]() ->bool {
			return this->stopped_.load() || !this->task_queue_.empty();
		});

		Task task;
		if (this->stopped_ && this->task_queue_.empty()) {
			return task;
		}

		task = std::move(task_queue_.front());
		task_queue_.pop();
		return task;
	}

	void schedule() {
		while (!this->stopped_) {
			if (Task task = get_one_task()) {
				task();
			}
		}
	}

private:
	std::vector<std::thread> thread_pool_;
	std::queue<Task>         task_queue_;
	std::mutex               mutex_;
	std::condition_variable  cond_var_;
	std::atomic<bool>        stopped_;
};

}
