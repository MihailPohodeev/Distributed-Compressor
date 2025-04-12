#include <DC_Interface_ConfigLoader.hxx>
#include <QueueParams.hxx>
#include <nlohmann/json.hpp>
#include <string>
#include <iostream>
#include <memory>

// strategy of receiving parameters from command line.
#include <queue_input_strategy/QueueInputStrategy.hxx>
#include <queue_input_strategy/RabbitMQ_QueueInputStrategy.hxx>
#include <queue_input_strategy/DC_Queue_QueueInputStrategy.hxx>

using json = nlohmann::json;

// constructor
DC_Interface_ConfigLoader::DC_Interface_ConfigLoader(std::string appName, std::string configFileName) : ConfigLoader(appName, configFileName) {}



// analize params.
void DC_Interface_ConfigLoader::params_analizer(int argc, char** argv, std::string& path, QueueParams* qp) const
{
	bool isPathFound = false;
	for (int i = 1; i < argc; i++)
	{
		std::string param(argv[i]);
		
		if (qp)
		{
			if (param == "-H" || param == "--host")
			{
				if (i == argc - 1)
					throw std::runtime_error(std::string("after ") + param + " you must to specify the hostname");
				std::string arg(argv[i + 1]);
				qp->host = arg;
				i++;
				continue;
			}
			else if (param == "-U" || param == "--user")
			{
				if (i == argc - 1)
					throw std::runtime_error(std::string("after ") + param + " you must to specify the username");
				std::string arg(argv[i + 1]);
				qp->username = arg;
				i++;
				continue;
			}
			else if (param == "-P" || param == "--password")
			{
				if (i == argc - 1)
					throw std::runtime_error(std::string("after ") + param + " you must to specify the password");
				std::string arg(argv[i + 1]);
				qp->password = arg;
				i++;
				continue;
			}
			else if (param == "-QT" || param == "--queue-type")
			{
				if (i == argc - 1)
					throw std::runtime_error(std::string("after ") + param + " you must to specify the type of queue [RabbitQM, DC-Queue]");
				std::string arg(argv[i + 1]);
				if (arg == "RabbitMQ")
				{
					qp->queueType = QueueType::RabbitMQ;
				}
				else if (arg == "DC-Queue")
				{
					qp->queueType = QueueType::DC_Queue;
				}
				else
					throw std::runtime_error(std::string("Unknow type of queue : ") + arg);
				i++;
				continue;
			}
		}
		if (param == "-RC" || param == "--ResetConfig")
		{
			remove_config_file();
		}
		else if (param == "-v" || param == "--version")
		{
			std::cout << "Distributed-Compressor version :\n\t1.0.0\n";
			exit(-1);
		}
		else if (is_valid_file_path(param))
		{
			if (isPathFound)
				throw std::runtime_error("Unknown parameter : " + param + "\n");
			isPathFound = true;
			path = param;
		}
		else
		{
			std::cerr << "Unknown parameter : " << param << '\n';
		}
	}
	if (!isPathFound)
		exit(-1);
}

// input parametars from command line.
void DC_Interface_ConfigLoader::input_parameters(QueueParams& qp) const
{
	// add here a new type of queue, if you create one.
	if (qp.queueType == QueueType::None)
	{
		std::cout << "Input queue type [RabbitMQ, DC-Input]: ";
		std::string queueType;
		std::cin >> queueType;

		if (queueType == "RabbitMQ")
		{
			qp.queueType = QueueType::RabbitMQ;
		}
		else if (queueType == "DC-Input")
		{
			qp.queueType = QueueType::DC_Queue;
		}
		else
		{
			throw std::runtime_error("Invalid queue type!\n");
		}
	}

	// add here option for you queue, if you create one.
	std::shared_ptr<QueueInputStrategy> inputStrategy = nullptr;

	if (qp.queueType == QueueType::RabbitMQ)
	{
		inputStrategy = std::make_shared<RabbitMQ_QueueInputStrategy>();
	}
	else if (qp.queueType == QueueType::DC_Queue)
	{
		inputStrategy = std::make_shared<DC_Queue_QueueInputStrategy>();
	}
	else
	{
		throw std::runtime_error("Invalid queue type!\n");
	}

	inputStrategy->handle_input(qp);
}
