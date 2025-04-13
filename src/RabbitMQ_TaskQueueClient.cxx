#include <RabbitMQ_TaskQueueClient.hxx>
#include <amqpcpp/libboostasio.h>
#include <amqpcpp.h>
#include <memory>
#include <string>
#include <boost/asio.hpp>
#include <boost/asio/steady_timer.hpp>
#include <chrono>


// constuctor.
RabbitMQ_TaskQueueClient::RabbitMQ_TaskQueueClient( boost::asio::io_context& io ) :
	TaskQueueClient(io),
	handler_(io_context_),
	connection_(nullptr),
	channel_(nullptr)
{}

// destructor.
RabbitMQ_TaskQueueClient::~RabbitMQ_TaskQueueClient() {
	std::lock_guard<std::mutex> lock(channel_mutex_);
	if (channel_) {
		channel_->close();
	}
	
	if (connection_) {
		connection_->close();
	}
}

// connect to the queue.
void RabbitMQ_TaskQueueClient::connect( const QueueParams& qp, std::function<void(bool)> callback )
{
	try {

	        std::string connectionString = "amqp://" + qp.username + ":" + qp.password + "@" + qp.host + "/";
        
		connection_ = std::make_shared<AMQP::TcpConnection>(&handler_, AMQP::Address(connectionString));
        
	    	channel_ = std::make_shared<AMQP::TcpChannel>(connection_.get());
        
	      	channel_->onError([callback](const char* message) {
	       		callback(false);
		});
		channel_->onReady([callback]() {
			callback(true);
	});
        
    	} catch (const std::exception& e) {
        	callback(false);
    	}
}

// disconnect from server.
void RabbitMQ_TaskQueueClient::disconnect( std::function<void()> callback )
{
	// close channel if it opened.
	{
		std::lock_guard<std::mutex> lock(channel_mutex_);
		if (channel_)
		{
			channel_->close();
			channel_.reset();
		}
	}

	// close connection if it opened.
	if (connection_) 
	{
		connection_->close();
		connection_.reset();
	}

	if (callback)
	{
		callback();
	}
}

// create new queue.
void RabbitMQ_TaskQueueClient::create_queue( const std::string& queueName, std::function<void(bool)> callback )
{
	ensure_connected([callback, this, queueName](bool connected) {
		if (!connected) {
			std::cout << "Connection isn't ready\n";
			if (callback)
				callback(false);
			return;
		}
		
		{
			std::lock_guard<std::mutex> lock(channel_mutex_);
			channel_->declareQueue(queueName, AMQP::durable)
			.onSuccess([callback]() {
					if (callback) callback(true);
				})
			.onError([callback](const char* message) {
				std::cerr << "Queue creation error: " << message << std::endl;
				if (callback)
					callback(false);
			});
		}
	});
}

// remove queue.
void RabbitMQ_TaskQueueClient::delete_queue(const std::string& queueName,
                                          std::function<void(bool)> callback)
{
	ensure_connected([this, queueName, callback](bool connected) {
		if (!connected) {
			std::cerr << "Cannot delete queue - not connected to RabbitMQ" << std::endl;
			if (callback) callback(false);
			return;
		}
		
		{
			std::lock_guard<std::mutex> lock(channel_mutex_);
			channel_->removeQueue(queueName)
			.onSuccess([callback]() {
				if (callback) callback(true);
			})
			.onError([callback](const char* message) {
				std::cerr << "Queue deletion error: " << message << std::endl;
				if (callback) callback(false);
			});
		}
	});
}

// push task to queue.
void RabbitMQ_TaskQueueClient::enqueue_task( const std::string& queueName, const std::string& taskBody, std::function<void(bool)> callback )
{
	ensure_connected([queueName, taskBody, this, callback](bool connected)
	{
		if (!connected) {
			if (callback) callback(false);
				return;
		}
		
		bool success = false;
		{
			std::lock_guard<std::mutex> lock(channel_mutex_);
			success = channel_->publish("", queueName, taskBody);
		}

		if (callback)
			callback(success);
	});
}

// subscribe to the queue.
void RabbitMQ_TaskQueueClient::subscribe( const std::string& queueName,
					  std::function<void(std::string)> messageCallback,
					  std::function<void()> subscribedCallback )
{
	ensure_connected([this, queueName, messageCallback, subscribedCallback](bool connected) {
		if (!connected) {
			if (subscribedCallback)
				subscribedCallback();
			return;
		}
		
		{
			std::lock_guard<std::mutex> lock(channel_mutex_);
			consumerTag_ = channel_->consume(queueName)
			.onReceived([this, messageCallback](const AMQP::Message& message, uint64_t deliveryTag, bool redelivered) 
			{
				std::string msg(message.body(), message.bodySize());
				this->channel_->ack(deliveryTag);
				if (messageCallback)
				{
					messageCallback(msg);
				}
			})
			.onError([messageCallback](const char* message) {
				std::cerr << "Subscribe error: " << message << std::endl;
			});
		}
	
		if (subscribedCallback) {
			subscribedCallback();
		}
	});
}

// unsubscribe from the queue.
void RabbitMQ_TaskQueueClient::unsubscribe( std::function<void()> callback )
{
	ensure_connected([this, callback](bool connected)
	{
		if (!connected || consumerTag_.empty()) {
			if (callback)
				callback();
			return;
		}

		{
			std::lock_guard<std::mutex> lock(channel_mutex_);
			channel_->cancel(consumerTag_)
			.onSuccess([callback]()
			{
				if (callback) 
					callback();
			})
			.onError([callback](const char* message)
			{
				std::cerr << "Unsubscribe error: " << message << std::endl;
				if (callback)
					callback();
			});
			consumerTag_.clear();
		}
	});
}

// check connection.
void RabbitMQ_TaskQueueClient::ensure_connected(std::function<void(bool)> callback)
{
	if (connection_ && connection_->ready()) {
            callback(true);
            return;
        }
        callback(false);
}
