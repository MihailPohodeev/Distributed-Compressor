#ifndef _TASK_POOL_HXX_
#define _TASK_POOL_HXX_

#include <functional>
#include <queue>
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>

// TASK_POOL is a class that allows to distribute tasks across free streams.

class TaskPool
{
	std::queue<std::function<void()>> tasksQueue_;	// queue for task saving.
	std::vector<std::thread> threads_;		// threads pool
	std::mutex queueMutex_;				// mutex for queue guarding.
	std::condition_variable queueCondition_;	// condition variable for to wake up the threads.
	std::atomic<bool> stopFlag_;			// flag for stopping the TaskPool.

public:
	// constructor.
	// threadsCount - is count of threads that would handle tasks.
	TaskPool(unsigned int threadsCount);
	// destructor.
	~TaskPool();

	// remove copy constuctor and operator=.
	TaskPool(const TaskPool&)  = delete;
	TaskPool& operator= ( const TaskPool& ) = delete;

	// define move-copy constructor and move-operator=.
	TaskPool(TaskPool&&) noexcept = default;
	TaskPool& operator=(TaskPool&&) noexcept = default;

	// add task to queue of tasks.
	void add_task( const std::function<void()>& task );
	void add_task( std::function<void()>&& task );

private:
	// function for every thread in pool.
	void worker_thread();
};

#endif
