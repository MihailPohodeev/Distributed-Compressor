#include <DC_Queue_TaskQueueClient.hxx>
#include <nlohmann/json.hpp>
#include <iostream>

using json = nlohmann::json;

DC_Queue_TaskQueueClient::DC_Queue_TaskQueueClient(boost::asio::io_context& io) : TaskQueueClient(io), dcQueueChannel_ptr( std::make_shared<DC_QueueChannel>(io) ) {}

void DC_Queue_TaskQueueClient::connect(  const QueueParams& qp, std::function<void(bool)> callback )
{
	std::lock_guard<std::mutex> lock(channelMutex_);
	dcQueueChannel_ptr->connect(qp.host, qp.port,
		[callback]()
		{
			if (callback)
				callback(true);
		},
		[callback]()
		{
			if (callback)
				callback(false);
		});
}

void DC_Queue_TaskQueueClient::disconnect( std::function<void()> callback)
{

}

void DC_Queue_TaskQueueClient::create_queue( const std::string& queueName, std::function<void(bool)> callback )
{
	std::lock_guard<std::mutex> lock(channelMutex_);
	dcQueueChannel_ptr->declare_queue( queueName,
		[callback]()
		{
			if (callback)
				callback(true);
		},
		[callback]()
		{
			if (callback)
				callback(false);
		});
}

void DC_Queue_TaskQueueClient::delete_queue(const std::string& queueName, std::function<void(bool)> callback)
{

}

void DC_Queue_TaskQueueClient::enqueue_task( const std::string& queueName, const std::string& taskBody, std::function<void(bool)> callback )
{
	std::lock_guard<std::mutex> lock(channelMutex_);
	dcQueueChannel_ptr->publish( queueName, taskBody, 
		[callback]()
		{
			if (callback)
				callback(true);
		},
		[callback]()
		{
			if (callback)
				callback(false);
		});
}

void DC_Queue_TaskQueueClient::subscribe( const std::string& queueName, std::function<void(std::string)> messageCallback, std::function<void()> subscribedCallback)
{
	dcQueueChannel_ptr->subscribe(queueName, messageCallback, nullptr);
}

void DC_Queue_TaskQueueClient::unsubscribe(std::function<void()> callback)
{

}
