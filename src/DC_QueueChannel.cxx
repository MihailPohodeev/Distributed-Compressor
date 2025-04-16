#include <DC_QueueChannel.hxx>
#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;
using namespace boost::asio;

DC_QueueChannel::DC_QueueChannel(boost::asio::io_context& io) : io_context_(io), socket_(io), currentGoalDescriptor_(1) {}

// send data to DC-Queue server.
void DC_QueueChannel::send_data(const std::string& data, std::function<void(bool)> callback)
{
	std::cout << "SEND DATA : " << data << '\n';
	std::vector<char> result(data.begin(), data.end() + 1);
	async_write(socket_, buffer(result), [callback, this](boost::system::error_code ec, size_t bytes) mutable
						{
							if (ec)
							{
								std::cerr << "Can't send data!\n";
								if (callback)
									callback(false);
								return;
							}
							if (callback)
								callback(true);
						});
}

void DC_QueueChannel::receive_data(std::function<void(bool, std::shared_ptr<json>)> callback)
{
	std::shared_ptr< std::vector<char> > buffer_ptr 		= std::make_shared< std::vector<char> >(1024);
	std::shared_ptr< std::vector<char> > accumulate_buffer_ptr	= std::make_shared< std::vector<char> >(0);

	socket_.async_read_some( buffer(*buffer_ptr), [this, buffer_ptr, accumulate_buffer_ptr, callback](const boost::system::error_code& ec, size_t bytes_transferred) mutable
	{
		if (!ec && bytes_transferred > 0)
		{
			accumulate_buffer_ptr->insert(accumulate_buffer_ptr->end(), buffer_ptr->begin(), buffer_ptr->begin() + bytes_transferred);
			auto null_pos = std::find(accumulate_buffer_ptr->begin(), accumulate_buffer_ptr->end(), '\0');

			while (null_pos != accumulate_buffer_ptr->end())
			{
				try
				{
					std::string json_str(accumulate_buffer_ptr->begin(), null_pos);
					std::shared_ptr<json> request = std::make_shared<json>( json::parse(json_str) );

					if (callback)
						callback(true, request);
				}
				catch (const json::parse_error& e)
				{
					if (callback)
						callback(false, nullptr);
					accumulate_buffer_ptr->clear();
				}

				auto next_pos = null_pos + 1;
				accumulate_buffer_ptr->erase(accumulate_buffer_ptr->begin(), next_pos);
				null_pos = std::find(accumulate_buffer_ptr->begin(), accumulate_buffer_ptr->end(), '\0');
			}
		}
		else if (ec != boost::asio::error::operation_aborted)
		{
			if (callback)
				callback(false, nullptr);
			return;
		}
		receive_data(callback);
	});
}

// add async task to the set.
unsigned int DC_QueueChannel::add_async_goal(std::function<void()> onSuccess, std::function<void()> onError, std::function<void(std::string)> onReceived)
{
	AsynchrousGoal ag(io_context_);
	ag.onSuccess = onSuccess;
	ag.onError = onError;
	ag.onReceived = onReceived;

	ag.timer->expires_after(std::chrono::seconds(5));

	unsigned int descriptor = currentGoalDescriptor_;

	if (!onReceived)
	{
		ag.timer->async_wait([descriptor, ag, this](const boost::system::error_code& ec)
			{
				if (!ec)
				{
					std::cout << "Timeout expired!\n";
					this->goals_.erase(currentGoalDescriptor_);
					boost::asio::post(io_context_, ag.onError);
				}
			});
	}

	{
		std::lock_guard<std::mutex> lock(goalsMutex_);
		goals_.insert( std::make_pair(currentGoalDescriptor_, ag) );
	}

	currentGoalDescriptor_++;
	return descriptor;
}

// connect to the DC-Queue-server.
void DC_QueueChannel::connect(const std::string& host, short port, std::function<void()> onSuccess, std::function<void()> onError)
{
	std::shared_ptr<ip::tcp::resolver> resolver = std::make_shared<ip::tcp::resolver>(io_context_);
	resolver->async_resolve(host, std::to_string(port), [this, resolver, onSuccess, onError](boost::system::error_code ec, ip::tcp::resolver::results_type endpoints)
		{
			if (ec)
			{
				std::cerr << "Can't resolve hosname-port!\n";
				if(onError)
					onError();
			}
			async_connect(socket_, endpoints, [this, onSuccess, onError](boost::system::error_code ec, ip::tcp::endpoint)
			{
				unsigned int goalDescriptor = add_async_goal(onSuccess, onError);

				json requestJSON;

				requestJSON["goal-descriptor"] 	= goalDescriptor;
				requestJSON["type"]		= "connection";

				send_data(requestJSON.dump());

				receive_data([this](bool success, std::shared_ptr<json> result)
				{
					if (!success)
					{
						std::cerr << "Can't handle response!\n";
						return;
					}
					std::cout << "Recieved response!\n";
					std::cout << "Income : " << result->dump() << '\n';
					try
					{					
						std::lock_guard<std::mutex> lock(goalsMutex_);
						unsigned int goalDescriptor	= result->at("goal-descriptor");
						AsynchrousGoal 	ag = goals_.at(goalDescriptor);
						ag.timer->cancel();
						goals_.erase(goalDescriptor);
						
				
						std::string typeOfRequest	= result->at("type");
						std::string status 		= result->at("status");

						if (typeOfRequest == "subscribe")
						{
							std::cout << "INCOME SUBSCRIBE PACKAGE!\n";
							std::string body	= result->at("body");
							if (status == "ok")
								if (ag.onReceived)
									boost::asio::post(io_context_, [ag, body](){ ag.onReceived(body); });
							else
								if (ag.onError)
									boost::asio::post(io_context_, ag.onError);
						}
						else
						{
							if (status == "ok")
								if (ag.onSuccess)
									boost::asio::post(io_context_, ag.onSuccess);
							else
								if (ag.onError)
									boost::asio::post(io_context_, ag.onError);
						}

					}
					catch(const json::type_error& e)
					{
						std::cerr << "Type error!\n" << e.what() << '\n';
					}
					catch(const json::out_of_range& e)
					{
						std::cerr << "Out of Range exception!\n" << e.what() << '\n';
					}
				});

			});
		});
}

// create new queue.
void DC_QueueChannel::declare_queue(const std::string& queueName, std::function<void()> onSuccess, std::function<void()> onError)
{
	unsigned int goalDescriptor = add_async_goal(onSuccess, onError);

	json requestJSON;

	requestJSON["goal-descriptor"]	= goalDescriptor;
	requestJSON["type"]		= "declare-queue";
	requestJSON["queue-name"]	= queueName;

	send_data(requestJSON.dump());
}

// remove queue.
void DC_QueueChannel::remove_queue(const std::string& queueName, std::function<void()> onSuccess, std::function<void()> onError)
{
	unsigned int goalDescriptor = add_async_goal(onSuccess, onError);

	json requestJSON;

	requestJSON["goal-descriptor"]	= goalDescriptor;
	requestJSON["type"]		= "remove-queue";
	requestJSON["queue-name"]	= queueName;

	send_data(requestJSON.dump());
}

// send new task to DC-Queue-server.
void DC_QueueChannel::publish(const std::string& queueName, const std::string& taskBody, std::function<void()> onSuccess, std::function<void()> onError)
{
	unsigned int goalDescriptor = add_async_goal(onSuccess, onError);

	json requestJSON;

	requestJSON["goal-descriptor"]	= goalDescriptor;
	requestJSON["type"]		= "publish";
	requestJSON["queue-name"]	= queueName;
	requestJSON["body"]		= taskBody;

	send_data(requestJSON.dump());
}

// subscribe on new task-queue in DC-Queue-server.
void DC_QueueChannel::subscribe(const std::string& queueName, std::function<void(std::string)> onReceived, std::function<void()> onError)
{
	unsigned int goalDescriptor = add_async_goal(nullptr, onError, onReceived);

	json requestJSON;

	requestJSON["goal-descriptor"]	= goalDescriptor;
	requestJSON["type"]		= "subscribe";
	requestJSON["queue-name"]	= queueName;

	send_data(requestJSON.dump());
}

// unsubscribe from specified task-queue.
void DC_QueueChannel::unsubscribe(const std::string& queueName, std::function<void()> onSuccess, std::function<void()> onError)
{
	unsigned int goalDescriptor = add_async_goal(onSuccess, onError);

	json requestJSON;

	requestJSON["goal-descriptor"]	= goalDescriptor;
	requestJSON["type"]		= "unsubscribe";
	requestJSON["queue-name"]	= queueName;

	send_data(requestJSON.dump());
}

void DC_QueueChannel::ack()
{
	json requestJSON;
	requestJSON["type"] = "ack";
	send_data(requestJSON.dump());
}
