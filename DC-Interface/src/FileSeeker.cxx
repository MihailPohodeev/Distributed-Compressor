#include <FileSeeker.hxx>
#include <iostream>

// constructor.
FileSeeker::FileSeeker()
{
	unsigned int threadsCount = std::max(1u, std::thread::hardware_concurrency() - 1);
	taskPool_ = std::make_unique<TaskPool>(threadsCount);
}

// recursively traversing directory and make action.
// dirpath - path to the directory to be processed.
// action  - a function that accepts a file path and performs an action on this file.
void FileSeeker::recursively_directory_action(const fs::path& dirpath, const std::function<void(const fs::path&)>& action)
{
	try
	{
		for ( const auto& entry : fs::recursive_directory_iterator(dirpath) )
		{
			if ( entry.is_regular_file() )
			{
				taskPool_->add_task( [action, path = entry.path()]() { action(path); } );
			}
		}
	}
	catch( const fs::filesystem_error& e )
	{
		std::cerr << "Error of directory access : " << e.what() << '\n';
	}
}
