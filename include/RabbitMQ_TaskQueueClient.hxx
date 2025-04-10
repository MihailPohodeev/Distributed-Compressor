#ifndef _RABBITMQ_TASK_QUEUE_CLIENT_HXX_
#define _RABBITMQ_TASK_QUEUE_CLIENT_HXX_

#include <string>
#include <memory>
#include <TaskQueueClient.hxx>

#include <amqpcpp.h>
#include <amqpcpp/linux_tcp.h>
#include <amqpcpp/libboostasio.h>


// RabbitMQ_TaskQueueClient is a class that implement communication with RabbitMQ-server.
class RabbitMQ_TaskQueueClient : public TaskQueueClient 
{
	AMQP::LibBoostAsioHandler handler_;			// handler for RabbitMQ communication though Boost::Asio.
	std::shared_ptr<AMQP::TcpConnection> connection_;	// connection to RabbitMQ.
	std::shared_ptr<AMQP::TcpChannel> channel_;		// channer for communication.
	std::string consumerTag_;
	std::mutex channel_mutex_;

	void ensure_connected(std::function<void(bool)> callback); // check, if connection doesn't exist.
public:
	// constructor.
	explicit RabbitMQ_TaskQueueClient( boost::asio::io_context& io );
	// destructor.
	~RabbitMQ_TaskQueueClient();

	void connect(  const std::string& hostname,const std::string& user,
                                const std::string& password, std::function<void(bool)> callback ) override;					// connect to queue-server.
        void disconnect( std::function<void()> callback = nullptr ) override;									// disconnect.
        void create_queue( const std::string& queueName, std::function<void(bool)> callback ) override;						// create new queue.
        void delete_queue(const std::string& queueName, std::function<void(bool)> callback) override;						// delete queue.
	void enqueue_task( const std::string& queueName, const std::string& taskBody, std::function<void(bool)> callback ) override;		// push task to queue.
	
	void subscribe( const std::string& queueName,
				std::function<void(std::string)> messageCallback,
				std::function<void()> subscribedCallback = nullptr) override;							// subscribe to a queue.
	void unsubscribe(std::function<void()> callback = nullptr) override;									// unsubscribe from the queue.

};

#endif
