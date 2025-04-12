#include <iostream>
#include <string>
#include <atomic>
#include <chrono>
#include <boost/filesystem.hpp>
#include <boost/asio.hpp>
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
boost::asio::io_context io;

std::chrono::seconds timeout = std::chrono::seconds(3);

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
			std::shared_ptr< boost::asio::steady_timer > timer	= std::make_shared< boost::asio::steady_timer >(io, timeout); // timer for canceling operation after inactivity.
			std::shared_ptr< std::atomic<int> > outgoingFiles 	= std::make_shared< std::atomic<int> >(0);		// number of outgoing files.
			std::shared_ptr< std::atomic<int> > incomingFiles 	= std::make_shared< std::atomic<int> >(0);		// number of incoming files.
			std::shared_ptr< std::atomic<bool>> isSendingFinished 	= std::make_shared< std::atomic<bool> >(false);		// is files outgoing fineshed -> 
																	// -> we can compare outgoing and incoming files.

			rq->subscribe( user_queue, [user_queue, outgoingFiles, incomingFiles, isSendingFinished, timer](std::string msg)
			{
				try
				{
					json msgJSON = json::parse(msg);
					std::string filepath = msgJSON.at("filepath");
					std::cout << std::string("got : ") + filepath + "\n";
					JSON_FilePacker::json_to_file(msgJSON);
				}
				catch(const json::parse_error& e)
				{
					std::cerr << "Parse Error!\n Can't handle message : " << msg << '\n';
				}
				catch (const json::type_error& e)
				{
					std::cerr << "Type Error!\nCan't handle task :" << msg << '\n';
				}
				catch (const json::out_of_range& e)
				{
					std::cerr << "Out of Range Error!\nCan't handle task : " << msg << '\n';
				}
				(*incomingFiles)++;
				timer->expires_after(timeout);

				if (*isSendingFinished)
				{
					if (*incomingFiles == *outgoingFiles)
					{
						timer->cancel();
						rq->delete_queue( user_queue, [](bool status) { io.stop(); } );
					}
				}

			}, nullptr );

			{
			        std::shared_ptr< FileSeeker > fs = std::make_shared< FileSeeker > ();
				
				fs->recursively_directory_action( directory, [rq, user_queue, outgoingFiles](const fs::path& filepath)
				{
					std::cout << std::string("file : ") + filepath.string() << '\n';
					json fileContent = JSON_FilePacker::file_to_json(filepath);
					fileContent["queue-id"] = user_queue;
					rq->enqueue_task( standardMainQueueName, fileContent.dump(), nullptr );
					(*outgoingFiles)++;
				});

				// here FileSeeker destroyed and all task in TaskPool inside FileSeeker finish work.
			}
			(*isSendingFinished) = true;

			timer->async_wait([rq, user_queue](const boost::system::error_code& ec)
			{
				// continue handling, if we restart without text message.
				// stop timer without text message, if cancelled.
				if (ec == boost::asio::error::operation_aborted)
					return;
				if (ec)
				{
					std::cerr << "timeout error!\n";
					exit(-1);
				}
				std::cout << "The waiting time has expired. Please try again.\n";
				rq->delete_queue( user_queue, [](bool status) { io.stop(); } );
			});
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

				fileHandling( directory, my_unique_domen );
                        });

		});
	});

	std::cout << "Start io.run()\n";
	io.run();

	return 0;
}
