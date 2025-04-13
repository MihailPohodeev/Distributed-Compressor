#include <TaskPool.hxx>
#include <iostream>

// constructor.
// threadsCount - is count of threads that would handle tasks.
TaskPool::TaskPool(unsigned int threadsCount) : stopFlag_(false)
{
	threads_.reserve(threadsCount);
	for (unsigned int i = 0; i < threadsCount; ++i) {
		threads_.emplace_back(&TaskPool::worker_thread, this);
	}
}

// destructor.
TaskPool::~TaskPool() {
	stopFlag_ = true;
	queueCondition_.notify_all();

	for (auto& thread : threads_) {
		if ( thread.joinable() ) {
			thread.join();
		}
	}
}

// add task to queue of tasks.
void TaskPool::add_task(const std::function<void()>& task) {
	{
		std::lock_guard<std::mutex> lock(queueMutex_);
		tasksQueue_.push(task);
	}
	queueCondition_.notify_one();
}

// add task to queue of tasks.
void TaskPool::add_task(std::function<void()>&& task) {
	{
		std::lock_guard<std::mutex> lock(queueMutex_);
		tasksQueue_.push(std::move(task));
	}
	queueCondition_.notify_one();
}

// handler of the tasks for each task.
void TaskPool::worker_thread() {
	while (true) {
		std::function<void()> task;
		{
			std::unique_lock<std::mutex> lock(queueMutex_);
			
			queueCondition_.wait(lock, [this]() {
					return !tasksQueue_.empty() || stopFlag_;
					});
	
			if (stopFlag_ && tasksQueue_.empty()) {
				return;
			}
			task = std::move(tasksQueue_.front());
			tasksQueue_.pop();
		}
		task();
	}
}
