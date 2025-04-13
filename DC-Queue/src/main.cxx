#include <boost/asio.hpp>
#include <iostream>

#include <ClientsManager.hxx>

using namespace boost::asio;

io_context io;

int main(int argc, char** argv)
{
	ClientsManager clientsManager(io);
	io.run();
	return 0;
}
