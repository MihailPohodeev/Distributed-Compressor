#include <ClientsManager.hxx>
#include <iostream>

ClientsManager::ClientsManager(io_context& io, short port) : io_context_(io), acceptor_(io, ip::tcp::endpoint(ip::tcp::v4(), port))
{
	std::cout << "Begin to accepting clients!\n";
	accept();	
}

// accept new clients.
void ClientsManager::accept()
{
	acceptor_.async_accept([this](boost::system::error_code ec, ip::tcp::socket socket)
			{
				if (!ec)
				{
					std::shared_ptr<Client> client_ptr = std::make_shared<Client>(std::move(socket));
					std::shared_ptr< std::function<void(bool, std::shared_ptr<json>)> > handle_ptr =
					std::make_shared< std::function<void(bool, std::shared_ptr<json>)> >
					(
						[handle_ptr, client_ptr](bool success, std::shared_ptr<json> requestJSON)
						{
							if (!success)
								std::cout << "failed! data can't be handled!\n";
							else
								std::cout << "success data receiving!\n";
							client_ptr->receive_data(*handle_ptr);
						}
					);

					client_ptr->receive_data(*handle_ptr);
					clients_.push_back( client_ptr );
				}
				accept();
			});
}
