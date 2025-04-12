#ifndef TASK_QUEUE_CLIENT_HXX
#define TASK_QUEUE_CLIENT_HXX

#include <string>
#include <memory>
#include <functional>
#include <boost/asio.hpp>

// Interface of task queue client for futher redefinition.
class TaskQueueClient
{
protected:
	boost::asio::io_context& io_context_;			// io_context for asynchronous operations.
public:
	// constructor.
	explicit TaskQueueClient( boost::asio::io_context& io );

	virtual void connect(	const std::string& hostname,const std::string& user,
				const std::string& password, std::function<void(bool)> callback ) = 0;						// connect to queue-server.
	virtual void disconnect( std::function<void()> callback = nullptr ) = 0;								// disconnect.
	virtual void create_queue( const std::string& queueName, std::function<void(bool)> callback ) = 0;					// create new queue.
	virtual void delete_queue(const std::string& queueName, std::function<void(bool)> callback) = 0;					// delete queue.
	virtual void enqueue_task( const std::string& queueName, const std::string& taskBody, std::function<void(bool)> callback ) = 0;		// push task to queue.

	virtual void subscribe(	const std::string& queueName,
	                  	std::function<void(std::string)> messageCallback,
        	          	std::function<void()> subscribedCallback = nullptr) = 0;							// subscribe to a queue.
	virtual void unsubscribe(std::function<void()> callback = nullptr) = 0;									// unsubscribe from the queue.
	
};

#endif
