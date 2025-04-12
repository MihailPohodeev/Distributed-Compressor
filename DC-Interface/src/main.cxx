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
#include <QueueParams.hxx>
#include <DC_Interface_ConfigLoader.hxx>

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

boost::asio::io_context io;

// timeout for automatic interruption after inactivity -> we didn't receive files during this timeout - interrupt handling.
std::chrono::seconds timeout = std::chrono::seconds(3);

int main( int argc, char** argv )
{
	// miminum 3 parameters.
	if (argc < 2)
	{
		printUsage( argv[0] );
		return 1;
	}
	std::string directory; // the directory where the files will be taken from.

	// --------------------------------------------------------------------------------
	// get parameters
	// It try to retreive parameters from config files, that can be found in ~/.config in Linux and in AppData in Windows.
	// If it doesn't exist -> retrieve this data from command line (interactive poll).
	QueueParams queueParams;
	std::unique_ptr<ConfigLoader> configLoader = std::make_unique<DC_Interface_ConfigLoader>("Distributed_Compressor");	// create loader of config-data.
	configLoader->params_analizer(argc, argv, directory, &queueParams);							// analize command line arguments.
	
	// try to load config-data from configuration-file ("config.json" default).
	bool isLoaded = configLoader->load_config(queueParams);

	// if we can't retrieve data from config-file:
	if (!isLoaded)
	{
		// interactive poll.
		configLoader->input_parameters(queueParams);
		// create new config-file with results of interactive poll.
		configLoader->save_config(queueParams);
	}
	// all data loaded -> remove config-loader.
	configLoader.reset();
	
	if (queueParams.queueType == QueueType::DC_Queue)
		throw std::runtime_error("DC_Queue communication not implemented!\n");
	// --------------------------------------------------------------------------------

	// we use random function for generating unique name for the queue from which we will extract the results.
	srand( time(0) );

	// task queue allows communicate with queues different types like RabbitMQ or DC-Queue.
	// TaskQueueClient - interface for specific implementation like RabbitMQ_TaskQueueClient of DC_Queue_TaskQueueClient.
	// create queue client for communication with Queue-server.
	std::shared_ptr< TaskQueueClient > queueClient;
	queueClient = std::make_shared< RabbitMQ_TaskQueueClient >(io);
	queueClient->connect(queueParams.host, queueParams.username, queueParams.password, [directory, queueClient](bool connected) {
		if (!connected) {
			std::cerr << "Failed to connect to RabbitMQ" << std::endl;
			exit(-1);
		}

		std::cout << "Successfuly connected to RabbitMQ" << std::endl;


		auto fileHandling = [queueClient](fs::path directory, std::string user_queue)
		{
			std::shared_ptr< boost::asio::steady_timer > timer	= std::make_shared< boost::asio::steady_timer >(io, timeout); // timer for canceling operation after inactivity.
			std::shared_ptr< std::atomic<int> > outgoingFiles 	= std::make_shared< std::atomic<int> >(0);		// number of outgoing files.
			std::shared_ptr< std::atomic<int> > incomingFiles 	= std::make_shared< std::atomic<int> >(0);		// number of incoming files.
			std::shared_ptr< std::atomic<bool>> isSendingFinished 	= std::make_shared< std::atomic<bool> >(false);		// is files outgoing fineshed -> 
																	// -> we can compare outgoing and incoming files.

			queueClient->subscribe( user_queue, [user_queue, outgoingFiles, incomingFiles, isSendingFinished, timer, queueClient](std::string msg)
			{
				try
				{
					json msgJSON = json::parse(msg);
					std::string filepath = msgJSON.at("filepath");
					std::cout << std::string("received : ") + filepath + "\n";
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
						queueClient->delete_queue( user_queue, [](bool status) { io.stop(); } );
					}
				}

			}, nullptr );

			{
			        std::shared_ptr< FileSeeker > fs = std::make_shared< FileSeeker > ();
				
				fs->recursively_directory_action( directory, [queueClient, user_queue, outgoingFiles](const fs::path& filepath)
				{
					std::cout << std::string("sent : ") + filepath.string() + "\n";
					json fileContent = JSON_FilePacker::file_to_json(filepath);
					fileContent["queue-id"] = user_queue;
					queueClient->enqueue_task( standardMainQueueName, fileContent.dump(), nullptr );
					(*outgoingFiles)++;
				});

				// here FileSeeker destroyed and all task in TaskPool inside FileSeeker finish work.
			}
			(*isSendingFinished) = true;

			timer->async_wait([queueClient, user_queue](const boost::system::error_code& ec)
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
				queueClient->delete_queue( user_queue, [](bool status) { io.stop(); } );
			});
		};

		queueClient->create_queue( standardMainQueueName, [queueClient, fileHandling, directory ](bool success) {
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

			queueClient->create_queue( my_unique_domen, [ queueClient, my_unique_domen, directory, fileHandling ](bool success) {
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

	io.run();

	return 0;
}
