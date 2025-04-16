#include <ClientsManager.hxx>
#include <iostream>

ClientsManager::ClientsManager(io_context& io, short port) :	io_context_(io),
								acceptor_(io, ip::tcp::endpoint(ip::tcp::v4(), port)),
								queueManager_( std::make_shared<QueueManager>() )
{
	std::cout << "Begin to accepting clients!\n";
	accept();	
}

// accept new clients.
void ClientsManager::accept()
{
	std::cout << "accept!\n";
	acceptor_.async_accept([this](boost::system::error_code ec, ip::tcp::socket socket)
			{
				accept();
				if (!ec)
				{
					std::shared_ptr<Client> client_ptr = std::make_shared<Client>(std::move(socket));
					client_ptr->receive_data( [client_ptr, this](bool success, std::shared_ptr<json> requestJSON)
						{
							if (!success)
							{
								std::cout << "failed! data can't be handled!\n";
								return;
							}
							std::cout << "success data receiving!\n";

							try
							{
								json responseJSON;

								//std::cout << "Request : " << requestJSON->dump() << '\n';

								if ( requestJSON->contains("goal-descriptor") )
									responseJSON["goal-descriptor"] = requestJSON->at("goal-descriptor");

								std::string type = requestJSON->at("type");

								responseJSON["type"] = type;

								if (type == "connection")
								{
									responseJSON["status"] = "ok";
								}
								else if (type == "declare-queue")
								{
									queueManager_->create_queue(requestJSON->at("queue-name"));
									responseJSON["status"] = "ok";
								}
								else if (type == "remove-queue")
								{
									queueManager_->remove_queue(requestJSON->at("queue-name"));
									responseJSON["status"] = "ok";
								}
								else if (type == "subscribe")
								{
									std::cout << "attempt to subscribe!\n";
									unsigned int descriptor = requestJSON->at("goal-descriptor");
									client_ptr->add_subscribe(requestJSON->at("queue-name"), descriptor);
									queueManager_->add_subscriber(requestJSON->at("queue-name"), client_ptr);
									responseJSON["status"] = "ok";
								}
								else if (type == "publish")
								{
									queueManager_->push_task(requestJSON->at("queue-name"), requestJSON->at("body"));
									responseJSON["status"] = "ok";
								}
								//std::cout << "\nResponse : " << responseJSON.dump() << '\n';
								if (type != "subscribe")
									client_ptr->send_data(responseJSON.dump());
							}
							catch(const json::type_error& e)
							{
								std::cerr << "Type error!\n" << e.what() << '\n';
							}
							catch(const json::out_of_range& e)
							{
								std::cerr << "Out of Range exception!\n" << e.what() << '\n';
							}
						}
					);
					clients_.push_back( client_ptr );
				}
			});
}
