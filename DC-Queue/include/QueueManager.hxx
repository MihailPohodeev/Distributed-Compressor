#ifndef QUEUE_MANAGER_HXX
#define QUEUE_MANAGER_HXX

#include <mutex>
#include <string>
#include <memory>
#include <unordered_map>

#include <Client.hxx>
#include <TaskQueue.hxx>

class QueueManager
{
	// mutex for guarding for hash-table.
	std::mutex mapMutex_;
	// hash-table for saving TaskQueues by name.
	std::unordered_map<std::string, std::shared_ptr<TaskQueue>> queues_;
public:
	// constructor
	QueueManager() = default;

	// -----------------------------------
	// remove unnesessary constructions.
	QueueManager(QueueManager&) = delete;
	QueueManager(QueueManager&&) = delete;

	QueueManager& operator=(QueueManager&) = delete;
	QueueManager& operator=(QueueManager&&) = delete;
	// -----------------------------------

	// -----------------------------------
	// this methonds change 'queues_' unordered_map content
	// we should guard this data structure.
	void create_queue(const std::string& queueName);
	void remove_queue(const std::string& queueName);
	// -----------------------------------
	
	// push new task in TaskQueue.
	bool push_task(const std::string& queueName, const std::string& task);

	bool add_subscriber(const std::string& queueName, std::shared_ptr<Client> client_ptr);
	bool remove_subscriber(const std::string& queueName, std::shared_ptr<Client> client_ptr);
	
};

#endif
