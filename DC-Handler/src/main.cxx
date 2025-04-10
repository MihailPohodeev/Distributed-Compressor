#include <iostream>
#include <string>
#include <boost/filesystem.hpp>
#include <TaskPool.hxx>
#include <RabbitMQ_TaskQueueClient.hxx>
#include <boost/asio.hpp>
#include <nlohmann/json.hpp>
#include <JSON_FilePacker.hxx>
#include <CompressHandler.hxx>

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

	std::shared_ptr<TaskExecutor> taskExecutor = std::make_shared<CompressorHandler>();

	rq->connect("sher-lock-find.ru", "kalich", "kal-kalich", [rq, taskExecutor](bool connected) {
		if (!connected) {
			std::cerr << "Failed to connect to RabbitMQ" << std::endl;
			exit(-1);
		}

		std::cout << "Successfuly connected to RabbitMQ" << std::endl;

		

		rq->subscribe( standardMainQueueName, [ taskExecutor ](std::string msg)
		{
			taskPool->add_task([msg, taskExecutor, rq]()
			{
				try
				{
					json msgJSON = json::parse(msg);
					// handle message 'msg'
					taskExecutor->handle(msgJSON, [rq](json response)
					{
						JSON_FilePacker::json_to_file(response);
					});
				}
				catch(const json::parse_error& e)
				{
					std::cerr << "Parse Error!\n Can't handle message : " << msg << '\n';
				}
			});
		});
	});

	io.run();
	return 0;
}
