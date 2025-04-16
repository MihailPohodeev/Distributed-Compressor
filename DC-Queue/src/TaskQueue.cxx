#include <TaskQueue.hxx>
#include <iostream>

// constructor.
TaskQueue::TaskQueue(const std::string& queueName) : name_(queueName), currentSubscriber_( subscribers_.begin() ) {}

// push new task in queue.
void TaskQueue::push_task(const std::string task)
{
	std::lock_guard<std::mutex> lock(queueMutex_);
	queue_.push(task);
}

// add subscribers.
void TaskQueue::add_subscriber(std::shared_ptr<Client> client_ptr)
{
	std::lock_guard<std::mutex> lock(subscribersMutex_);
	subscribers_.push_back(client_ptr);
	if (currentSubscriber_ == subscribers_.end())
	{
		currentSubscriber_ = subscribers_.begin();
	}
}

// remove subscribers.
void TaskQueue::remove_subscriber(std::shared_ptr<Client> client_ptr)
{
	std::lock_guard<std::mutex> lock(subscribersMutex_);

	if (client_ptr.get() == (*currentSubscriber_).get())
	{
		++currentSubscriber_;
	}

	subscribers_.remove(client_ptr);
}

// send tasks from queue to subscribers.
void TaskQueue::notify_subscribers()
{
	std::lock_guard<std::mutex> queLock(queueMutex_);
	std::lock_guard<std::mutex> subLock(subscribersMutex_);

	std::cout << "QUEUE SIZE : " << queue_.size() << " ; SUBS : " << subscribers_.size() << '\n';

	while (!queue_.empty() && subscribers_.size() > 0)
	{
		if (currentSubscriber_ == subscribers_.end())
			currentSubscriber_ = subscribers_.begin();

		if (currentSubscriber_ == subscribers_.end())
			break;

		std::string task = queue_.front();
		queue_.pop();

		json responseJSON;

		responseJSON["type"] 		= "subscribe";
		responseJSON["queue-name"]	= name_;
		responseJSON["body"]		= task;
		responseJSON["status"]		= "ok";
		
		unsigned int descriptor = (*currentSubscriber_)->get_subscribe(name_);
		responseJSON["goal-descriptor"] = descriptor;

		std::cout << "NOTIFY USERS SUBSCR : " << responseJSON.dump() << '\n';

		(*currentSubscriber_)->send_data(responseJSON.dump());
	//	(*currentSubscriber_)->add_task(task);
		++currentSubscriber_;
	}
}

