#ifndef FILE_SEEKER_HXX
#define FILE_SEEKER_HXX

#include <string>
#include <boost/filesystem.hpp>
#include <memory>		// unique_ptr for taskPool_.
#include <functional>		// for functors and functions - actions for files.
#include <TaskPool.hxx>

namespace fs = boost::filesystem;

// FILE-SEEKER is class for recursively searching files in directory and making some action with this files.

class FileSeeker
{
	std::unique_ptr<TaskPool> taskPool_; // task pool for task allocation.
public:
	// constructor
	FileSeeker();
	
	// recursively traversing directory and make action.
	// dirpath - path to the directory to be processed.
	// action  - a function that accepts a file path and performs an action on this file.
	void recursively_directory_action(const fs::path& dirpath, const std::function<void(const fs::path&)>& action);	
};

#endif
