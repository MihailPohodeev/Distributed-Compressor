#ifndef QUEUE_INPUT_STRATEGY_HXX
#define QUEUE_INPUT_STRATEGY_HXX

#include <QueueParams.hxx>

class QueueInputStrategy
{
public:
	/* handle input for this queue-type
	 *
	 * For instance, if queue require just host - it require input host
	 * If queue require login and password - it require login and password
	 *
	 */
	virtual void handle_input(QueueParams& qp) const = 0;
};

#endif
