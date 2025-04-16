#ifndef CLIENT_HXX
#define CLIENT_HXX

#include <string>
#include <mutex>
#include <queue>
#include <functional>
#include <unordered_map>
#include <boost/asio.hpp>
#include <nlohmann/json.hpp>

using namespace boost::asio;
using json = nlohmann::json;

class Client
{
	ip::tcp::socket socket_;
	std::queue<std::string> queueTasks_;
	std::map<std::string, unsigned int> subscribeDescriptors_;

	mutable std::mutex queueTasksMutex_;
	mutable std::mutex socket_sending_mutex_;
	mutable std::mutex socket_receiving_mutex_;
	mutable std::mutex mapSubscribesMutex_;
	
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

	// push new task to queue with tasks.
	void add_task(const std::string& task);
	// get top of task queue.
	std::string top_task();
	// pop task from queue after acknowledge.
	void pop_task();
	// add subscribe to the map with descriptor value.
	void add_subscribe(const std::string& queueName, unsigned int subscribeDescriptor);
	// get descriptor of client and specified queue.
	unsigned int get_subscribe(const std::string& queueName);
	// remove descriptor from table.
	void remove_subscribe(const std::string& queueName);
};

#endif
