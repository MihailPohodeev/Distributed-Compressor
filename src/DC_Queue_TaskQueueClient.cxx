#include <DC_Queue_TaskQueueClient.hxx>
#include <nlohmann/json.hpp>
#include <iostream>

using json = nlohmann::json;

DC_Queue_TaskQueueClient::DC_Queue_TaskQueueClient(boost::asio::io_context& io) : TaskQueueClient(io), socket_(io) {}

void DC_Queue_TaskQueueClient::connect(  const QueueParams& qp, std::function<void(bool)> callback )
{
	std::shared_ptr<ip::tcp::resolver> resolver = std::make_shared<ip::tcp::resolver>(io_context_);
	resolver->async_resolve(qp.host, std::to_string(qp.port), [this, resolver, callback](boost::system::error_code ec, ip::tcp::resolver::results_type endpoints)
		{
			if (ec) {
				std::cerr << "Can't resolve hosname-port!\n";
				callback(false);
			}
			async_connect(socket_, endpoints, [this, callback](boost::system::error_code ec, ip::tcp::endpoint)
				{
					if (!ec)
						callback(true);
					else
						callback(false);
				});
		});
}

void DC_Queue_TaskQueueClient::disconnect( std::function<void()> callback)
{

}

void DC_Queue_TaskQueueClient::create_queue( const std::string& queueName, std::function<void(bool)> callback )
{
	json responseJSON;
	responseJSON["command"] 	= "create-queue";
	responseJSON["queue-name"]	= queueName;

	send_data(responseJSON.dump());
	
	if(callback)
		callback(true);
}

void DC_Queue_TaskQueueClient::delete_queue(const std::string& queueName, std::function<void(bool)> callback)
{

}

void DC_Queue_TaskQueueClient::enqueue_task( const std::string& queueName, const std::string& taskBody, std::function<void(bool)> callback )
{

}

void DC_Queue_TaskQueueClient::subscribe( const std::string& queueName, std::function<void(std::string)> messageCallback, std::function<void()> subscribedCallback)
{

}

void DC_Queue_TaskQueueClient::unsubscribe(std::function<void()> callback)
{

}

void DC_Queue_TaskQueueClient::send_data(const std::string& data)
{
	std::shared_ptr< std::lock_guard<std::mutex> > lock_ptr = std::make_shared< std::lock_guard<std::mutex> >(socket_mutex_);
	async_write(socket_, buffer(data), [lock_ptr, this](boost::system::error_code ec, size_t bytes)
						{
							if (ec)
							{
								std::cerr << "Can't send data!\n";
								return;
							}
						});
}
