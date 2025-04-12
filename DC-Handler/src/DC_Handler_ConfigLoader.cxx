#include <DC_Handler_ConfigLoader.hxx>
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

// constructor.
DC_Handler_ConfigLoader::DC_Handler_ConfigLoader(std::string appName, std::string configFileName) : ConfigLoader(appName, configFileName) {}

// analize parameters.
void DC_Handler_ConfigLoader::params_analizer(int argc, char** argv, std::string& path, QueueParams* qp) const
{
	if (qp == nullptr)
		throw std::runtime_error("qp pointer is nullable in DC_Handler_ConfigLoader::params_analizer!\n");

        for (int i = 1; i < argc; i++)
        {
                std::string param(argv[i]);

                if (param == "-H" || param == "--host")
                {
			if (i == argc - 1)
				throw std::runtime_error(std::string("after ") + param + " you must to specify the hostname");
			std::string arg = argv[i + 1];
			qp->host = arg;
			i++;
                }
		else if (param == "-QT" || param == "--queue-type")
		{
			if (i == argc - 1)
				throw std::runtime_error(std::string("after ") + param + " you must to specify the hostname");
			std::string arg = argv[i + 1];
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
		}
		else if (param == "-U" || param == "--user")
		{
			if (i == argc - 1)
                                throw std::runtime_error(std::string("after ") + param + " you must to specify the username");
			std::string arg = argv[i + 1];
			qp->username = arg;
			i++;
		}
		else if(param == "-P" || param == "--password")
		{
			if (i == argc - 1)
                                throw std::runtime_error(std::string("after ") + param + " you must to specify the password");
			std::string arg = argv[i + 1];
			qp->password = arg;
			i++;
		}
                else if (param == "-v" || param == "--version")
                {
                        std::cout << "Distributed-Compressor-Handler version :\n\t1.0.0\n";
                        exit(-1);
                }
                else
                {
                        std::cerr << "Unknown parameter : " << param << '\n';
			exit(-1);
                }
        }
}

// input parametars from command line.
void DC_Handler_ConfigLoader::input_parameters(QueueParams& qp) const
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
