#include <Client.hxx>
#include <iostream>
#include <memory>

// constructor.
Client::Client(ip::tcp::socket socket) : socket_(std::move(socket)) {}


void Client::send_data(const std::string& data, std::function<void(bool)> callback)
{
	std::shared_ptr< std::lock_guard<std::mutex> > lock_ptr = std::make_shared< std::lock_guard<std::mutex> >(socket_sending_mutex_);
	async_write(socket_, buffer(data), [this, callback, lock_ptr](boost::system::error_code ec, size_t bytes)
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
	std::shared_ptr< std::vector<char> > accumulate_buffer_ptr	= std::make_shared< std::vector<char> >(1024);

	socket_.async_read_some( buffer(*buffer_ptr), [this, buffer_ptr, accumulate_buffer_ptr, lock_ptr, callback](const boost::system::error_code& ec, size_t bytes_transferred)
	{
		if (!ec && bytes_transferred > 0)
		{
			std::cout << "Income : " << std::string( buffer_ptr->begin(), buffer_ptr->end() ) + "\n";
			/*
			auto& buf = *read_buffer_;
			size_t search_start = buf.size() - bytes_transferred;
	
			auto begin = buf.begin() + search_start;
			auto end = buf.end();

			while (true) 
			{
				auto null_pos = std::find(begin, end, '\0');
		
				if (null_pos == end)
				{
					accumulation_buffer_.insert(accumulation_buffer_.end(), begin, end);
					break;
				}
		
				accumulation_buffer_.insert( accumulation_buffer_.end(), begin, null_pos);
		
				if (!accumulation_buffer_.empty()) {
					callback_({accumulation_buffer_.begin(), accumulation_buffer_.end()});
					accumulation_buffer_.clear();
				}
				begin = null_pos + 1;
			}
			buf.erase(buf.begin(), begin);
			*/
		}
		else if (ec != boost::asio::error::operation_aborted)
		{
			if (callback)
				callback(false, nullptr);
		}
	});
}
