#ifndef RABBIT_MQ_QUEUE_INPUT_STRATEGY_HXX
#define RABBIT_MQ_QUEUE_INPUT_STRATEGY_HXX

#include <stdexcept>
#include <queue_input_strategy/QueueInputStrategy.hxx>

/*
 *	Implementation of QueueInputStrategy for RabbitMQ-queue.
 *	It allows receive nesessary parameters in interactive mode.
 */

class RabbitMQ_QueueInputStrategy : public QueueInputStrategy
{
public:
	void handle_input(QueueParams& qp) const override
	{
		std::string inputString;
		if (qp.host == "")
		{
			std::cout << "Input hostname : ";
			std::cin >> inputString;
			if (inputString == "")
				throw std::runtime_error("Invalid hostname!\n");
			qp.host = inputString;
		}

		if (qp.username == "")
		{
			inputString = "";
			std::cout << "Input username(login) : ";
			std::cin >> inputString;
			if (inputString == "")
				throw std::runtime_error("Invalid username(login)!\n");
			qp.username = inputString;
		}

		if (qp.password == "")
		{
			inputString = "";
			std::cout << "Input password : ";
			std::cin >> inputString;
			if (inputString == "")
				throw std::runtime_error("Invalid username(login)!\n");
			qp.password = inputString;
		}
	}
};

#endif
