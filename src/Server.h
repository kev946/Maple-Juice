/*
 * Server.h
 *
 *  Created on: Jun 29, 2013
 *      Author: klpeter2
 */

#ifndef SERVER_H_
#define SERVER_H_

#include "MembershipList.h"
#include "MessageCreator.h"
#include "TCPClient.h"
#include "FileUtils.h"
#include "FileList.h"

#include <set>
#include <map>
#include <string>
#include <iostream>
#include <fstream>
#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/ref.hpp>
#include <boost/thread.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>
#include <boost/algorithm/string/regex.hpp>

class Server
{
public:
	Server(boost::asio::io_service& ioService, const std::string& ip);
	virtual ~Server();
	void start();
	void stop();
	void setIntroducerIp(const std::string& ip);
	std::string getIntroducerIp();

	void getFile(const std::string& fileName, const std::string& sdfsFileName);
	void putFile(const std::string& fileName,  const std::string& sdfsFileName);
	void deleteFile(const std::string& sdfsFileName);
	void maple(std::vector<std::string>& args);
	void juice(std::vector<std::string>& args);
	void processJuiceMaster(std::vector<std::string> args);
	void processJuice(std::vector<std::string> args);

private:
	void startReceive();
	void handleReceive(const boost::system::error_code& error,
			std::size_t bytes_transferred);
	void ping(const std::string& ip, const std::string& message);

	void addToMembershipList(MembershipList::tuple& message);
	void listCaseHelper(std::string& message);
	void pingServers();
	void checkForLeaves();
	void pingFileList();

	MembershipList membershipList;
	MessageCreator messageCreator;
	FileUtils fileUtils;
	FileList fileList;

	bool running;
	std::string introducerNodeIp;
	std::string myIp;
	std::string introducerIp;
	int myPort;
	std::ofstream log;

	boost::asio::ip::udp::socket socket;
	boost::asio::ip::udp::endpoint remoteEndpoint;
	boost::array<char, 256> recvBuf;

	TCPClient tcpClient;
};

#endif /* SERVER_H_ */
