/*
 * FileUtils.h
 *
 *  Created on: Jul 13, 2013
 *      Author: klpeter2
 */

#ifndef FILEUTILS_H_
#define FILEUTILS_H_

#include <fstream>
#include <boost/asio.hpp>
#include <iostream>
#include <vector>

class FileUtils
{
public:
	FileUtils();
	virtual ~FileUtils();
	void sendFile(const std::string& fileName,
			boost::asio::ip::tcp::socket& socket);
	bool fileExists(const std::string& fileName);
	void open(const std::string& fileName);
	bool isOpen(const std::string& fileName);
	void close(const std::string& fileName);
	void write(const std::string& fileName, const std::string& data);
	static int generateFile(const std::string& fileName, int size);
	static int removeFile(const std::string& fileName);
private:
	typedef std::pair<std::string, std::ofstream> pair;
	std::vector<pair> files;
};

#endif /* FILEUTILS_H_ */
