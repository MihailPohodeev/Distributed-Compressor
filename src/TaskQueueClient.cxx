#include <TaskQueueClient.hxx>

// constructor.
TaskQueueClient::TaskQueueClient(boost::asio::io_context& io) : io_context_(io) {}
