#ifndef QUEUE_PARAMS_HXX
#define QUEUE_PARAMS_HXX

#include <string>

// type of queue.
enum class QueueType { None, RabbitMQ, DC_Queue };

/*
 *	Struct for loading queue-parametes from different functions.
 */

struct QueueParams
{
	QueueType queueType;					// type of queue [RabbitMQ, DC-Queue];
	std::string username;					// username for RabbitMQ;
	std::string password;					// password for RabbitMQ;
	std::string host;					// host of Queue (hostname or ip);
	QueueParams() : queueType(QueueType::None){}		// constructor.
};

#endif
