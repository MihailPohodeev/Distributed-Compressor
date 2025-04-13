#ifndef CLIENT_HXX
#define CLIENT_HXX

#include <string>
#include <mutex>
#include <queue>
#include <functional>
#include <boost/asio.hpp>
#include <nlohmann/json.hpp>

using namespace boost::asio;
using json = nlohmann::json;

class Client
{
	ip::tcp::socket socket_;
	std::queue<std::string> queueTasks_;

	mutable std::mutex socket_sending_mutex_;
	mutable std::mutex socket_receiving_mutex_;

public:
	// constructor.
	Client(ip::tcp::socket socket);

	// -----------------------------------
	// remove unnesessary constructions.
	Client(Client&)  		= delete;
	Client(Client&&) 		= delete;
	Client operator=(Client&) 	= delete;
	Client operator=(Client&&)	= delete;
	// -----------------------------------
	
	// handle input/output of client.

	void send_data(const std::string& data, std::function<void(bool)> callback = nullptr);
	void receive_data(std::function<void(bool, std::shared_ptr<json>)> callback = nullptr);
};

#endif
