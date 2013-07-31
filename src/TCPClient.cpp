#include "TCPClient.h"

#include <string>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

TCPClient::TCPClient(const std::string& ip) : myIp(ip)
{
		boost::thread(&TCPClient::listen, this);
}

TCPClient::~TCPClient()
{

}

void TCPClient::query(const std::string& ipToQuery, const std::string& message, const std::string& fileName)
{
try
  {
		// Read message
		std::vector<std::string> result;
		boost::split(result, message, boost::is_any_of(" "));
		std::string command = result[0];
		std::string ip = result[1];
		std::string sdfsFileName = result[2];

		boost::asio::io_service io_service;
//
    tcp::resolver resolver(io_service);

    tcp::resolver::query query(ipToQuery, "55520"); 

    tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
    tcp::resolver::iterator end;
    tcp::socket socket(io_service);
    boost::system::error_code error = boost::asio::error::host_not_found;
//
    while (error && endpoint_iterator != end)
    {
      socket.close();
      socket.connect(*endpoint_iterator++, error);
    }

    if (error)
      throw boost::system::system_error(error);

		boost::system::error_code ignored_error;
	  boost::asio::write(socket, boost::asio::buffer(message),
    boost::asio::transfer_all(), ignored_error);
		
		// Open file for writing
		std::fstream fs;
		fs.open(fileName.c_str(), std::fstream::out);
		//std::cout << "Copying file " << sdfsFileName << " to " << fileName << std::endl;

    for (;;)
    {
      boost::array<char, 128> buf;
      boost::system::error_code error;

      size_t len = socket.read_some(boost::asio::buffer(buf), error);

      if (error == boost::asio::error::eof)
        break; // Connection closed cleanly by peer.
      else if (error)
        throw boost::system::system_error(error); // Some other error.

			// Write data to file
			std::string line(buf.c_array(), len);
			fs << line;
    }

		// Close file
		fs.close();
  }
  catch (std::exception& e)
  {
    std::cerr << e.what() << std::endl;
  }
}

void TCPClient::listen()
{
	while(true)
	{
		  try
			{
				boost::asio::io_service io_service;

				tcp::acceptor acceptor(io_service, tcp::endpoint(tcp::v4(), 55520));

				for (;;)
				{
					boost::array<char, 128> buf;
					boost::system::error_code error;
				  tcp::socket socket(io_service);
				  acceptor.accept(socket);
					
				  size_t len = socket.read_some(boost::asio::buffer(buf), error);
					
					std::string message(buf.c_array(), len);					

					std::vector<std::string> result;
					boost::split(result, message, boost::is_any_of(" "));
					std::string ip = result[1];
					std::string fileName = result[2];

					fileUtils.sendFile(fileName, boost::ref(socket));
				}
			}
			catch (std::exception& e)
			{
				std::cerr << e.what() << std::endl;
			}
	}
}
