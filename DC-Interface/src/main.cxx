#include <iostream>
#include <string>
#include <boost/filesystem.hpp>
#include <FileSeeker.hxx>
#include <TaskPool.hxx>
#include <RabbitMQ_TaskQueueClient.hxx>
#include <boost/asio.hpp>
#include <nlohmann/json.hpp>
#include <JSON_FilePacker.hxx>

namespace fs = boost::filesystem;
using json = nlohmann::json;

// the name of main task queue.
const std::string standardMainQueueName = "Main-Task-Queue";

void printUsage(const std::string& programName) {
    std::cerr << "Usage: " << programName << " <directory> <action-argument>\n"
              << "Recursively processes files in <directory> with specified action.\n"
              << "Example:\n"
              << "  " << programName << " /path/to/dir print\n";
}

fs::path get_executable_path() {
	try
	{
		return fs::canonical("/proc/self/exe").parent_path();
	}
	catch (...) 
	{
		return fs::current_path();
	}
}

std::shared_ptr< TaskQueueClient > rq;
boost::asio::io_context io;

int main( int argc, char** argv )
{
	if (argc < 2)
	{
		printUsage( argv[0] );
		return 1;
	}
	srand( time(0) );

	const std::string directory = argv[1];

	rq = std::make_shared< RabbitMQ_TaskQueueClient >(io);

	rq->connect("sher-lock-find.ru", "kalich", "kal-kalich", [directory, rq](bool connected) {
		if (!connected) {
			std::cerr << "Failed to connect to RabbitMQ" << std::endl;
			exit(-1);
		}

		std::cout << "Successfuly connected to RabbitMQ" << std::endl;


		auto fileHandling = [rq](fs::path directory, std::string user_queue)
		{	
			{
			        std::shared_ptr< FileSeeker > fs = std::make_shared< FileSeeker > ();
				
				fs->recursively_directory_action( directory, [rq](const fs::path& filepath)
				{
					std::cout << "file : " << filepath << '\n';
					rq->enqueue_task( standardMainQueueName, filepath.string(), nullptr);
					//json fileContent = JSON_FilePacker::file_to_json(filepath);
					//fileContent["queue-id"] = my_unique_domen;
					
					//std::cout << fileContent.dump();
				});
			}
			rq->delete_queue( user_queue, [](bool status) { io.stop(); } );
		};

		rq->create_queue( standardMainQueueName, [ fileHandling, directory ](bool success) {
			if (!success)
			{
                        	std::cerr << "Can't create main queue : " << standardMainQueueName << '\n';
				exit(-1);
			}
			
			std::cout << "Successfuly created new main queue : " << standardMainQueueName << '\n';

			std::string my_unique_domen =   boost::asio::ip::host_name() +
							fs::current_path().string() + 
							directory +
							std::to_string(rand()) + "_queue";

			rq->create_queue( my_unique_domen, [ my_unique_domen, directory, fileHandling ](bool success) {
				if (!success)
				{
					std::cerr << "Can't create unique queue : " << my_unique_domen << '\n';
					exit(-1);
				}
				
				std::cout << "Successfuly created new unique queue : " << my_unique_domen << '\n';

				fileHandling(directory, my_unique_domen );
                        });

		});
	});

	std::cout << "Start io.run()\n";
	io.run();

	return 0;
}
