/*
 * TCPClient.h
 *
 *  Created on: Jul 14, 2013
 *      Author: klpeter2
 */

#ifndef TCPCLIENT_H_
#define TCPCLIENT_H_

#include "FileUtils.h"
#include "FileList.h"

#include <string>
#include <iostream>
#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/thread.hpp>
#include <boost/algorithm/string/regex.hpp>
#include <boost/algorithm/string.hpp>
#include <fstream>

class TCPClient
{
public:
	TCPClient(const std::string& ip);
	virtual ~TCPClient();
	void query(const std::string& ipToQuery, const std::string& message, const std::string& fileName);
	void listen();

private:
	boost::asio::io_service io_service;
	std::string myIp;
	FileUtils fileUtils;
};

#endif /* TCPCLIENT_H_ */
