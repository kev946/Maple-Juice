/*
 * FileUtils.cpp
 *
 *  Created on: Jul 13, 2013
 *      Author: klpeter2
 */

#include "FileUtils.h"

FileUtils::FileUtils()
{
	// TODO Auto-generated constructor stub

}

FileUtils::~FileUtils()
{
	// TODO Auto-generated destructor stub
}

/*
 * Returns true if a file with given name exists
 * in the local directory, false if it doesn't
 */
bool FileUtils::fileExists(const std::string& fileName)
{
	std::ifstream ifile(fileName.c_str());
	return ifile;
}

/**
 * Send the contents of a file over socket
 */
void FileUtils::sendFile(const std::string& fileName,
		boost::asio::ip::tcp::socket& socket)
{
	if (fileExists(fileName))
	{
		//std::cout << "sending file " << fileName << std::endl;
		std::string line;
		std::ifstream infile;
		infile.open(fileName.c_str());
		while (!infile.eof())
		{
			std::getline(infile, line);
			line += "\n";
			boost::system::error_code ignored_error;
			boost::asio::write(socket, boost::asio::buffer(line),
					boost::asio::transfer_all(), ignored_error);
		}
		infile.close();
	}
}

void FileUtils::open(const std::string& fileName)
{
	std::ofstream outputFile;
	outputFile.open(fileName.c_str());
	std::cout << "file opened" << std::endl; //TODO:
}

bool FileUtils::isOpen(const std::string& fileName)
{
	for (std::vector<FileUtils::pair>::iterator it = files.begin();
			it != files.end(); it++)
	{
		// matching key
		if (it->first == fileName)
		{
			bool isopen = it->second.is_open();
			return isopen;
		}
	}
	return false;
}

void FileUtils::close(const std::string& fileName)
{
	if (isOpen(fileName))
	{
		for (std::vector<FileUtils::pair>::iterator it = files.begin();
				it != files.end(); it++)
		{
			// matching key
			if (it->first == fileName)
			{
				it->second.close();
				it->first = ""; //TODO: For some reason, we can't vector::erase the iterator (private??)
			}
		}
	}
}

void FileUtils::write(const std::string& fileName, const std::string& data)
{
	if (isOpen(fileName))
	{
		for (std::vector<FileUtils::pair>::iterator it = files.begin();
				it != files.end(); it++)
		{
			// matching key
			if (it->first == fileName)
			{
				it->second.write(data.c_str(), sizeof(data));
			}
		}
	}
}

int FileUtils::generateFile(const std::string& fileName, int size)
{
	FILE * file;
	file = fopen(fileName.c_str(), "w");

	int b_size = 0, count = 0;
	char * buffer = (char *) malloc((1024 * 256) + 1);

	while (b_size < (1024 * 256))
	{ //creates the buffer we write to log
		strcat(buffer, "Garbage\n");
		b_size += 8;
	}

	while (1)
	{ //writes to disk in chunks to avoid overhead
		if (count == 4)
		{
			size--;

			if (size == 0)
				break;

			count = 0;
		}
		count++;

		fwrite(buffer, sizeof(char), 1024 * 256, file);
	}

	free(buffer);
	fclose(file);

	return 1;

}

int FileUtils::removeFile(const std::string& name)
{

	FILE * fil = fopen(name.c_str(), "r");
	if (fil == NULL)
		return -1; //file does not exist

	remove(name.c_str()); //file exists

	return 1;

}
