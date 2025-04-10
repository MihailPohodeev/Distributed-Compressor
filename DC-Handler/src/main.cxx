#include <iostream>
#include <string>
#include <boost/filesystem.hpp>
#include <TaskPool.hxx>
#include <RabbitMQ_TaskQueueClient.hxx>
#include <boost/asio.hpp>
#include <nlohmann/json.hpp>
#include <JSON_FilePacker.hxx>

namespace fs = boost::filesystem;
using json = nlohmann::json;

std::shared_ptr< TaskQueueClient > rq;
std::shared_ptr< TaskPool > taskPool;
boost::asio::io_context io;

const std::string standardMainQueueName = "Main-Task-Queue";

int main()
{
	rq = std::make_shared< RabbitMQ_TaskQueueClient >(io);

	unsigned int threadsCount = std::max(1u, std::thread::hardware_concurrency() - 1);
	taskPool = std::make_shared< TaskPool > (threadsCount);

	rq->connect("sher-lock-find.ru", "kalich", "kal-kalich", [rq](bool connected) {
		if (!connected) {
			std::cerr << "Failed to connect to RabbitMQ" << std::endl;
			exit(-1);
		}

		std::cout << "Successfuly connected to RabbitMQ" << std::endl;
		rq->subscribe( standardMainQueueName, [](std::string msg)
		{
			taskPool->add_task([msg]()
			{
				std::cout << msg << '\n';
			});
		} );
	});

	io.run();
	return 0;
}
