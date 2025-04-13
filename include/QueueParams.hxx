#ifndef QUEUE_PARAMS_HXX
#define QUEUE_PARAMS_HXX

#include <string>

#define STANDARD_DC_QUEUE_PORT 15651


// type of queue.
enum class QueueType { None, RabbitMQ, DC_Queue };

/*
 *	Struct for loading queue-parametes from different functions.
 */

struct QueueParams
{
	QueueType 	queueType;				// type of queue [RabbitMQ, DC-Queue];
	std::string 	username;				// username for RabbitMQ;
	std::string 	password;				// password for RabbitMQ;
	std::string 	host;					// host of Queue (hostname or ip);
	short 		port;					// port of Queue (for DC-Queue)
	QueueParams() : queueType(QueueType::None), port(STANDARD_DC_QUEUE_PORT) {} // constructor.
};

#endif
