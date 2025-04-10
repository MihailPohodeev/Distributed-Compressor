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

std::shared_ptr< TaskQueueClient > rq;

int main( int argc, char** argv )
{
	if (argc < 2)
	{
		printUsage( argv[0] );
		return 1;
	}
	srand( time(0) );

	const std::string directory = argv[1];

	boost::asio::io_context io;

	rq = std::make_shared< RabbitMQ_TaskQueueClient >(io);

	rq->connect("sher-lock-find.ru", "kalich", "kal-kalich", [&](bool connected) {
		if (!connected) {
			std::cerr << "Failed to connect to RabbitMQ" << std::endl;
			exit(-1);
		}

			std::cout << "Successfuly connected to RabbitMQ" << std::endl;

			std::string my_unique_domen =	boost::asio::ip::host_name() +
							fs::current_path().string() +
							directory +
							std::to_string(rand());

			std::cout << my_unique_domen << '\n';

			auto fileHandling = [&](const fs::path& directory)
			{
				std::mutex tmp;	
					
			        std::unique_ptr< FileSeeker > fs = std::make_unique< FileSeeker > ();

				fs->recursively_directory_action( directory, [&](const fs::path& filepath)
				{
					std::lock_guard<std::mutex> lock(tmp);
					std::cout << "file : " << filepath << '\n';
					json fileContent = JSON_FilePacker::file_to_json(filepath);
					std::cout << fileContent.dump() << '\n';
					
					/*
			                rq->enqueue_task( standardMainQueueName, filepath, [](bool success)
					{
						if (!success)
							std::cerr << "Can't send data to queue!\n";
					});
					*/
					JSON_FilePacker::json_to_file(fileContent);
			        });

				//rq->subscribe(standardMainQueueName, [](std::string str) { std::cout << "got : " << str << '\n'; });
				
			};

			rq->is_queue_exist( standardMainQueueName, [&](bool success){
				if (!success)
				{
					rq->create_queue( standardMainQueueName, [&](bool success) {
						if (!success)
							std::cerr << "Can't create main queue : " << standardMainQueueName << '\n';
						else
						{
							std::cout << "Successfuly created new main queue : " << standardMainQueueName << '\n';

							fileHandling(directory);
						}
					});
				}
				else
				{
					std::cout << "Main Queue already exist!\n";
					fileHandling(directory);
				}
			});
		});

	io.run();

	return 0;
}
