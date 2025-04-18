#ifndef TASK_EXECUTOR_HXX
#define TASK_EXECUTOR_HXX

#include <functional>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

/*	 Interface for executing task in DC-Handler
 * 	It allows override task-handling in subclasses and perform different actions on a task-messages.
 */

class TaskExecutor
{
public:
	virtual void handle(const json& task, std::function<void(json)> callback = nullptr) = 0;		// handle one task and after it - make a callback
};

#endif
