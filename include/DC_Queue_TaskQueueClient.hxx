#ifndef DC_QUEUE_TASK_QUEUE_CLIENT_HXX
#define DC_QUEUE_TASK_QUEUE_CLIENT_HXX

// standard port for connection to DC-Queue.
#define STANDARD_DC_QUEUE_PORT 15651

#include <mutex>
#include <string>
#include <memory>
#include <boost/asio.hpp>
#include <TaskQueueClient.hxx>
#include <QueueParams.hxx>

using namespace boost::asio;

// DC_Queue_TaskQueueClient is class that allows to communicate with DC-Queue server.
class DC_Queue_TaskQueueClient : public TaskQueueClient
{
	ip::tcp::socket socket_;

	mutable std::mutex socket_mutex_;		// guard socket from racecondition.

	void send_data(const std::string& data);
public:
	// constructor.
	explicit DC_Queue_TaskQueueClient(boost::asio::io_context& io);

	void connect( const QueueParams& qp, std::function<void(bool)> callback ) override;					// connect to queue-server.
	void disconnect( std::function<void()> callback = nullptr ) override;									// disconnect.
	void create_queue( const std::string& queueName, std::function<void(bool)> callback ) override;						// create new queue.
	void delete_queue(const std::string& queueName, std::function<void(bool)> callback) override;						// delete queue.
	void enqueue_task( const std::string& queueName, const std::string& taskBody, std::function<void(bool)> callback ) override;		// push task to queue.

	void subscribe( const std::string& queueName,
				std::function<void(std::string)> messageCallback,
				std::function<void()> subscribedCallback = nullptr) override;							// subscribe to a queue.
	void unsubscribe(std::function<void()> callback = nullptr) override;									// unsubscribe from the queue.

};

#endif
