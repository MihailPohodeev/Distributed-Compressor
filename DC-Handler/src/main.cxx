#include <iostream>
#include <memory>
#include <string>
#include <boost/filesystem.hpp>
#include <TaskPool.hxx>
#include <RabbitMQ_TaskQueueClient.hxx>
#include <DC_Queue_TaskQueueClient.hxx>
#include <boost/asio.hpp>
#include <nlohmann/json.hpp>
#include <JSON_FilePacker.hxx>
#include <CompressHandler.hxx>
#include <DC_Handler_ConfigLoader.hxx>

namespace fs = boost::filesystem;
using json = nlohmann::json;

std::shared_ptr< TaskPool > taskPool;
boost::asio::io_context io;

const std::string standardMainQueueName = "Main-Task-Queue";

int main(int argc, char** argv)
{
	// --------------------------------------------------------------------------------
	// receive parameters for handler work
	QueueParams queueParams;
	std::unique_ptr<ConfigLoader> configLoader = std::make_unique<DC_Handler_ConfigLoader>("Distributed_Handler");
	std::string path;
	configLoader->params_analizer(argc, argv, path, &queueParams);
	configLoader->input_parameters(queueParams);
	configLoader.reset();
	// --------------------------------------------------------------------------------

	// create queue-client for communication with queue-server.
	std::shared_ptr< TaskQueueClient > queueClient;
	
	if (queueParams.queueType == QueueType::RabbitMQ)
		queueClient = std::make_shared< RabbitMQ_TaskQueueClient >(io);
	else if (queueParams.queueType == QueueType::DC_Queue)
		queueClient = std::make_shared< DC_Queue_TaskQueueClient >(io);
	else
		std::runtime_error("Invalid Queue-type!\n");


	// create task-pool with threads = logical cores of proccessor - 1 of 1.
	unsigned int threadsCount = std::max(1u, std::thread::hardware_concurrency() - 1);
	taskPool = std::make_shared< TaskPool > (threadsCount);

	// create taskExecutor for Compressing incoming files.
	std::shared_ptr<TaskExecutor> taskExecutor = std::make_shared<CompressorHandler>();

	// connect to the queue-server.
	queueClient->connect(queueParams, [queueClient, taskExecutor](bool connected) {
		if (!connected) {
			std::cerr << "Failed to connect to Queue-Server!\n";
			exit(-1);
		}

		std::cout << "Successfuly connected to Queue-Server!\n";		

		// subscribe on main-task queue.
		queueClient->subscribe( standardMainQueueName, [queueClient, taskExecutor](std::string msg)
		{
			// add new task for processing.
			taskPool->add_task([msg, taskExecutor, queueClient]()
			{
				try
				{
					json msgJSON = json::parse(msg);
					std::string queueID = msgJSON["queue-id"];
					std::string queueFilepath = msgJSON.at("filepath");
					std::cout << std::string("processing : ") + queueFilepath + "\n";
					
					// handle message 'msg'
					taskExecutor->handle(msgJSON, [queueClient, queueID](json response)
					{
						queueClient->enqueue_task(queueID, response.dump(), nullptr);
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
