/*
 * FileList.h
 *
 *  Created on: Jul 13, 2013
 *      Author: klpeter2
 */

#ifndef FILELIST_H_
#define FILELIST_H_

#include <vector>
#include <boost/tuple/tuple.hpp>
#include <boost/algorithm/string/regex.hpp>
#include <boost/algorithm/string.hpp>
#include <iostream>
#include <string>
#include <algorithm>

class FileList
{
public:

	typedef boost::tuple<std::string, std::vector<std::string> > tuple;

	FileList();
	virtual ~FileList();
	void add(tuple tup);
	void removeFile(const std::string& fileName);
	bool isInList(const std::string& fileName);
	void print();
	std::vector<tuple> get();
	std::string tupleToString(const tuple& tup);
	int size();
	std::string getIp(const std::string& fileName);
	void removeIp(const std::string& ip);
	std::string ownersOfFile(const std::string& fileName);
	void update(std::string fileName, std::string ip);
	int numOwners(const std::string& fileName);
	static std::string myIpStatic;

private:
	std::vector<tuple> fileList;
};

#endif /* FILELIST_H_ */
