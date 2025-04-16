#ifndef TASK_QUEUE_HXX
#define TASK_QUEUE_HXX

#include <list>
#include <string>
#include <queue>
#include <mutex>
#include <memory>
#include <Client.hxx>

class TaskQueue
{
	std::string name_;							// name of queue.

	std::mutex queueMutex_;							// mutex for safe queue using.
	std::mutex subscribersMutex_;						// mutex for safe subscribers_ list using.
	
	std::queue<std::string> queue_;						// queue with tasks.

	std::list<std::shared_ptr<Client>> subscribers_;			// subscribers that will receive tasks from this queue.
										// Use std::list because std::vector can hava invalid iterators.

	std::list<std::shared_ptr<Client>>::iterator currentSubscriber_;	// current iterator of subscribers_ list. We send tasks from queue to client using algorithm round-robin.

public:
	// constructor.
	TaskQueue(const std::string& queueName);

	// push new task in queue.
	void push_task(const std::string task);

	// add and remove subscribers.
	void add_subscriber(std::shared_ptr<Client> client_ptr);
	void remove_subscriber(std::shared_ptr<Client> client_ptr);

	// send tasks from queue to subscribers.
	void notify_subscribers();
};

#endif
