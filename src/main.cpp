/*
 * main.cpp
 *
 *  Created on: Jun 29, 2013
 *      Author: klpeter2
 */

#include "Server.h"
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <vector>
#include <string>

/*
 * IP address of the introducer process.
 *
 * This is the process that allows other
 * processes to join the group!
 *
 */
const std::string INTROCUCER_IP = "linux6.ews.illinois.edu";

void maple(std::vector<std::string>& args, Server& server)
{
	if(args.size() < 4)
	{
		std::cout << "Usage: " << "maple <maple_exe> <sdfs_inter_filename_prefix> <sdfs_src_filename_1> ... <sdfs_src_filename_m>" << std::endl;
		std::cout << "Please try again." << std::endl;
	}
	else
	{
		boost::thread t(boost::bind(&Server::maple, boost::ref(server), args));
	}
}
void juice(std::vector<std::string>& args, Server& server)
{
	if(args.size() != 5)
	{
		std::cout << "Usage: " << "juice <juice_exe> <num_juices> <sdfs_inter_filename_prefix> <sdfs_dest_filename>" << std::endl;
		std::cout << "Please try again." << std::endl;
	}
	else
	{
		boost::thread t(boost::bind(&Server::juice, boost::ref(server), args));
	}
}
/*
 * Get input commands from the user
 * Commands are: put <filename>, get <filename>, and delete <filename>
 *
 */
void getInput(Server& server)
{
	std::string input;
	std::string command;
	std::string fileName;
	std::vector<std::string> results;
	while (true)
	{
		std::getline(std::cin, input);
		boost::split(results, input, boost::is_any_of(" "),
				boost::token_compress_on);
		command = results[0];
		if(command == "maple")
		{
			maple(results, server);
		}			
		else if(command == "juice")
		{
			juice(results, server);
		}
		else if(command != "put" && command != "get" && command != "delete"
				&& command != "generatefile" && command != "removefile")
		{

		}
		else
		{
			if (command == "put")
			{
				if (results.size() == 3)
				{
					std::string fileName = results[1];
					std::string sdfsFileName = "/tmp/" + results[2];
					boost::thread t(
							boost::bind(&Server::putFile, boost::ref(server),
									fileName, sdfsFileName));
				}
				else
				{
					std::cout << "Invalid command." << std::endl;
				}
			}
			else if (command == "get")
			{
				if (results.size() == 3)
				{
					std::string fileName = results[2];
					std::string sdfsFileName = "/tmp/" + results[1];
					boost::thread t(
							boost::bind(&Server::getFile, boost::ref(server),
									fileName, sdfsFileName));
				}
				else
				{
					std::cout << "Invalid command." << std::endl;
				}
			}
			else if (command == "delete")
			{
				if (results.size() == 2)
				{
					std::string sdfsFileName = "/tmp/" + results[1];
					boost::thread t(
							boost::bind(&Server::deleteFile, boost::ref(server),
									sdfsFileName));
				}
				else
				{
					std::cout << "Invalid command." << std::endl;
				}
			}

			else if (command == "generatefile")
			{
				std::cout << "Enter filename" << std::endl;
				std::string name;
				std::cin >> name;
				std::cout << "Enter file size im MB" << std::endl;
				int size = 0;
				std::cin >> size;
				std::cout << "Generating file..." << std::endl;
				int x = FileUtils::generateFile(name, size);
				if (x == 1)
				{
					std::cout << "Success!" << std::endl;
				}
				else
				{
					std::cout << "Failure!" << std::endl;
				}
			}
			else if (command == "removefile")
			{
				std::cout << "Enter filename" << std::endl;
				std::string name;
				std::cin >> name;
				std::cout << "Removing file..." << std::endl;
				int x = FileUtils::removeFile(name);
				if (x == 1)
				{
					std::cout << "Success!" << std::endl;
				}
				else
				{
					std::cout << "Failure!" << std::endl;
				}
			}
			else
			{
				// Nothing
			}
		}
	}
}
/*
 * Entry point to our program
 */
int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		std::cout << "Usage: This machines name <ip>" << std::endl;
		return 1;
	}

// Create server
	boost::asio::io_service ioService;
	Server server(ioService, argv[1]);
	server.setIntroducerIp(INTROCUCER_IP);
	boost::thread inputThread(getInput, boost::ref(server)); // Start polling for user input commands
	server.start();
	ioService.run();

	return 0;
}

