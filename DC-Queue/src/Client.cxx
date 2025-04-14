#include <Client.hxx>
#include <iostream>
#include <memory>

// constructor.
Client::Client(ip::tcp::socket socket) : socket_(std::move(socket)) {}


void Client::send_data(const std::string& data, std::function<void(bool)> callback)
{
	std::shared_ptr< std::lock_guard<std::mutex> > lock_ptr = std::make_shared< std::lock_guard<std::mutex> >(socket_sending_mutex_);
	std::vector<char> result(data.begin(), data.end() + 1);
	async_write(socket_, buffer(result), [this, callback, lock_ptr](boost::system::error_code ec, size_t bytes)
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
	std::shared_ptr< std::lock_guard<std::mutex> > lock_ptr 	= std::make_shared< std::lock_guard<std::mutex> >(socket_receiving_mutex_);
	std::shared_ptr< std::vector<char> > buffer_ptr 		= std::make_shared< std::vector<char> >(1024);
	std::shared_ptr< std::vector<char> > accumulate_buffer_ptr	= std::make_shared< std::vector<char> >(0);

	socket_.async_read_some( buffer(*buffer_ptr), [this, buffer_ptr, accumulate_buffer_ptr, lock_ptr, callback](const boost::system::error_code& ec, size_t bytes_transferred)
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
					std::cerr << "Can't parse json : " << e.what() << '\n';
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
		}
	});
}
