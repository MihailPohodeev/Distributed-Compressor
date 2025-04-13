#ifndef CLIENTS_MANAGER_HXX
#define CLIENTS_MANAGER_HXX

#include <list>
#include <memory>
#include <boost/asio.hpp>

#include <Client.hxx>

#define STANDARD_DC_QUEUE_PORT 15651

using namespace boost::asio;

class ClientsManager
{
	boost::asio::io_context& io_context_;
	std::list<std::shared_ptr<Client>> clients_;

	ip::tcp::acceptor acceptor_;

	// accepting new clients.
	void accept();
public:
	ClientsManager(io_context& io, short port = STANDARD_DC_QUEUE_PORT);
};

#endif
