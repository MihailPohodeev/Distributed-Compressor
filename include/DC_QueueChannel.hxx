#ifndef DC_QUEUE_CHANNEL_HXX
#define DC_QUEUE_CHANNEL_HXX

#include <map>
#include <functional>
#include <nlohmann/json.hpp>
#include <boost/asio.hpp>

using json = nlohmann::json;
using namespace boost::asio;

struct AsynchrousGoal
{
	std::function<void()> onSuccess;
	std::function<void(std::string)> onReceived;
	std::function<void()> onError;
	std::shared_ptr<steady_timer> timer;

	AsynchrousGoal(boost::asio::io_context& io) : timer( std::make_shared<steady_timer>(io) ) {}
};

class DC_QueueChannel
{
	// map of asynchronous goals.
	// unsigned int - goal descriptor;
	// AsynchrousGoal - goal;
	std::map<unsigned int, AsynchrousGoal> goals_;

	// io_context for asyncronous communication.
	boost::asio::io_context& io_context_;

	// guard socket from race-condition.
	mutable std::mutex socket_mutex_;

	mutable std::mutex goalsMutex_;

	// socket for communication with server.
	ip::tcp::socket socket_;

	// current descriptor of task.
	unsigned int currentGoalDescriptor_;

	// send data to server.
	void send_data(const std::string& data, std::function<void(bool)> callback = nullptr);
	// receive data from server.
	void receive_data(std::function<void(bool, std::shared_ptr<json>)> callback);
	// create new async goal.
	unsigned int add_async_goal(std::function<void()> onSuccess, std::function<void()> onError, std::function<void(std::string)> onReceived = nullptr);
public:
	// constructor.
	DC_QueueChannel(boost::asio::io_context& io);

	// connect to the DC-Queue-server.
	void connect(const std::string& host, short port, std::function<void()> onSuccess, std::function<void()> onError);
	// create new queue.
	void declare_queue(const std::string& queueName, std::function<void()> onSuccess, std::function<void()> onError);
	// remove queue.
	void remove_queue(const std::string& queueName, std::function<void()> onSuccess, std::function<void()> onError);
	// send new task to DC-Queue-server.
	void publish(const std::string& queueName, const std::string& taskBody, std::function<void()> onSuccess, std::function<void()> onError);
	// subscribe on new task-queue in DC-Queue-server.
	void subscribe(const std::string& queueName, std::function<void(std::string)> onReceived, std::function<void()> onError);
	// unsubscribe from specified task-queue.
	void unsubscribe(const std::string& queueName, std::function<void()> onSuccess, std::function<void()> onError);
	// acknowledge result.
	void ack();
};

#endif
