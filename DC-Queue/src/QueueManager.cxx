#include <QueueManager.hxx>
#include <iostream>

// create new queue.
void QueueManager::create_queue(const std::string& queueName)
{
	std::cout << "Created task-queue with name : " << queueName << '\n';
	std::lock_guard<std::mutex> lock(mapMutex_);
	auto it = queues_.find(queueName);
	if ( it != queues_.end() )
		return;
	queues_[queueName] = std::make_shared<TaskQueue>(queueName);
}

// remove queue.
void QueueManager::remove_queue(const std::string& queueName)
{
	std::lock_guard<std::mutex> lock(mapMutex_);
	queues_.erase(queueName);
}

// push new task into specified TaskQueue.
bool QueueManager::push_task(const std::string& queueName, const std::string& task)
{
	std::lock_guard<std::mutex> lock(mapMutex_);
	auto it = queues_.find(queueName);
	if (it == queues_.end())
		return false;
	it->second->push_task(task);
	it->second->notify_subscribers();
	return true;
}

// add subscriber to queue.
bool QueueManager::add_subscriber(const std::string& queueName, std::shared_ptr<Client> client_ptr)
{
	std::lock_guard<std::mutex> lock(mapMutex_);
	auto it = queues_.find(queueName);
	if (it == queues_.end())
		return false;
	it->second->add_subscriber(client_ptr);
	it->second->notify_subscribers();
	return true;
}

// remove subscriber from queue.
bool QueueManager::remove_subscriber(const std::string& queueName, std::shared_ptr<Client> client_ptr)
{
	std::lock_guard<std::mutex> lock(mapMutex_);
	auto it = queues_.find(queueName);
	if (it == queues_.end())
		return false;
	it->second->remove_subscriber(client_ptr);
	return true;
}
