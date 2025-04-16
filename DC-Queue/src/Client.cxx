#include <Client.hxx>
#include <iostream>
#include <memory>

// constructor.
Client::Client(ip::tcp::socket socket) : socket_(std::move(socket)) {}


void Client::send_data(const std::string& data, std::function<void(bool)> callback)
{
	std::vector<char> result(data.begin(), data.end() + 1);
	async_write(socket_, buffer(result), [this, callback](boost::system::error_code ec, size_t bytes) mutable
			{
				if (callback)
				{
					if (ec)
						callback(false);
					else
						callback(true);
				}
			});
}

void Client::receive_data(std::function<void(bool, std::shared_ptr<json>)> callback)
{
	std::shared_ptr< std::vector<char> > buffer_ptr 		= std::make_shared< std::vector<char> >(4096);
	std::shared_ptr< std::vector<char> > accumulate_buffer_ptr	= std::make_shared< std::vector<char> >(0);

	socket_.async_read_some( buffer(*buffer_ptr), [this, buffer_ptr, accumulate_buffer_ptr, callback](const boost::system::error_code& ec, size_t bytes_transferred) mutable
	{
		if (!ec && bytes_transferred > 0)
		{
			accumulate_buffer_ptr->insert(accumulate_buffer_ptr->end(), buffer_ptr->begin(), buffer_ptr->begin() + bytes_transferred);
			auto null_pos = std::find(accumulate_buffer_ptr->begin(), accumulate_buffer_ptr->end(), '\0');
			//std::cout << "\n\nReceived : " << std::string(accumulate_buffer_ptr->begin(), accumulate_buffer_ptr->end()) << '\n';
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

void Client::add_task(const std::string& task)
{
	std::lock_guard<std::mutex> lock(queueTasksMutex_);
	queueTasks_.push(task);
}

std::string Client::top_task()
{
	std::lock_guard<std::mutex> lock(queueTasksMutex_);
	return queueTasks_.front();
}

void Client::pop_task()
{
	std::lock_guard<std::mutex> lock(queueTasksMutex_);
	queueTasks_.pop();
}

// add subscribe to the map with descriptor value.
void Client::add_subscribe(const std::string& queueName, unsigned int subscribeDescriptor)
{
	std::lock_guard<std::mutex> lock(mapSubscribesMutex_);
	subscribeDescriptors_[queueName] = subscribeDescriptor;
}

// get descriptor of client and specified queue.
unsigned int Client::get_subscribe(const std::string& queueName)
{
	std::lock_guard<std::mutex> lock(mapSubscribesMutex_);
	auto it = subscribeDescriptors_.find(queueName);
	if (it == subscribeDescriptors_.end())
		return 0;
	return it->second;
}

// remove descriptor from table.
void Client::remove_subscribe(const std::string& queueName)
{
	std::lock_guard<std::mutex> lock(mapSubscribesMutex_);
	subscribeDescriptors_.erase(queueName);
}
