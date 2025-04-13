#ifndef QUEUE_MANAGER_HXX
#define QUEUE_MANAGER_HXX

#include <string>
#include <memory>
#include <unordered_map>

#include <Client.hxx>
#include <TaskQueue.hxx>

class QueueManager
{
	std::unordered_map<std::string, std::shared_ptr<TaskQueue>> queues_
public:
	// constructor
	QueueManager();

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
	void add_queue(const std::string& queueName);
	void remove_queue(const std::string& queueName);
	// -----------------------------------
	
	void add_subscriber(const std::string& queueName, std::shared_ptr<Client> client);
	void remove_subscriber(const std::string& queueName, std::shared_ptr<Client> client);
	
};

#endif
