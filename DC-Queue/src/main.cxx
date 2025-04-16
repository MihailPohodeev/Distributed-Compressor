#include <boost/asio.hpp>
#include <iostream>

#include <ClientsManager.hxx>

using namespace boost::asio;

io_context io;

int main(int argc, char** argv)
{
	std::unique_ptr<ClientsManager> clientsManager = std::make_unique<ClientsManager>(io);
	io.run();
	return 0;
}
