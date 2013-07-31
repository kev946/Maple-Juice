/*
 * Server.cpp
 *
 *  Created on: Jun 29, 2013
 *      Author: klpeter2
 */

#include "Server.h"

using boost::asio::ip::udp;

/*
 * Construct the server.
 * Running initially set to false.
 *
 */
Server::Server(boost::asio::io_service& ioService_, const std::string& ip) :
		socket(ioService_, udp::endpoint(udp::v4(), 55555)), myIp(ip), tcpClient(ip)
{
	running = false;
	myPort = 55555;
	std::string logName = myIp + ".log";
	log.open(logName.c_str());
	time_t time = std::time(NULL);
	FileList::myIpStatic = ip;
	log << "(ip, timestamp, port) Created at " << time << std::endl;
}

Server::~Server()
{

}

/*
 * Start the server.
 * Ping the introducer process as well.
 *
 */
void Server::start()
{
	if (!running)
	{
		running = true;
		startReceive();
	}

	std::string message = messageCreator.message(2, myIp, myPort);
	boost::thread serverThread = boost::thread(
			boost::bind(&Server::ping, this,
					introducerIp /* introducer process */, message));
	boost::thread pingThread = boost::thread(
			boost::bind(&Server::pingServers, this));
	boost::thread leaveThread = boost::thread(
			boost::bind(&Server::checkForLeaves, this));
	boost::thread fileListThread = boost::thread(
			boost::bind(&Server::pingFileList, this));
}

/*
 * Listen in the background for a new request (ping).
 * If received, the io_service object invokes the handleReceive method.
 *
 */
void Server::startReceive()
{
	try
	{
		socket.async_receive_from(boost::asio::buffer(recvBuf), remoteEndpoint,
				boost::bind(&Server::handleReceive, this,
						boost::asio::placeholders::error,
						boost::asio::placeholders::bytes_transferred));

	} catch (std::exception& e)
	{
		running = false;
		std::cerr << e.what() << std::endl;
	}
}

/*
 * Invoked when a message is received from a process (callback method).
 * *** This is where we decide what to do with the message received ***
 *
 * @param error Indicates whether the operation succeeded or failed
 * @param bytesTransferred Number of bytes received
 */
void Server::handleReceive(const boost::system::error_code& error,
		std::size_t bytesTransferred)
{
	// Parse message we received
	std::string message(recvBuf.c_array(), bytesTransferred);
	std::vector<std::string> result;
	boost::split(result, message, boost::is_any_of("_"));
	int action = boost::lexical_cast<int>(result[0]);
	std::string ip = result[1];
	time_t timestamp = std::time(NULL);
	if (action != 4 && action != 5 && action != 6 && action != 7 && action != 8)
		timestamp = boost::lexical_cast<time_t>(result[2]);
	MembershipList::tuple tuple(ip, timestamp, 55555);

	std::string msg;
	switch (action)
	{
	case 0: // Received a ping, send an ack
		msg = messageCreator.message(1, myIp, myPort);

		// New member joined
		if (!membershipList.isInList(ip))
		{
			membershipList.add(tuple);
			std::string s = membershipList.tupleToString(tuple);
			log << s << " added to membership list." << std::endl;
		}
		ping(ip, msg);
		break;
	case 1: // Received an ack. Update membership list timestamp
		membershipList.update(tuple);
		break;
	case 2: // JOIN
		addToMembershipList(tuple);
		msg = messageCreator.message(membershipList.get(), myIp, myPort);
		ping(ip, msg); // Send back membership list
		break;
	case 3: // LIST (list being received)
		if (ip == myIp)
			break;
		else
		{
			// Update our membership list
			listCaseHelper(message);
			break;
		}
	case 4: // PUT FILE REQUEST
	{
					std::string sdfsFileName = result[3];
		// If not on process, then request file
		if (!fileUtils.fileExists(result[3]))
		{
			// Request File if we don't have
			std::string fileName = result[2];
			std::string message = "0 " + ip + " " + fileName;
			if(!fileUtils.fileExists(sdfsFileName))
				tcpClient.query(ip, message, sdfsFileName);
		
		}
		// Add to File List
		fileList.update(sdfsFileName, myIp);
		fileList.update(sdfsFileName, ip);
		break;
	}
	case 5: // UPDATE FILE LIST
	{
		std::string fileName = result[2];
		for(int i = 4; i < result.size(); i++)
		{
			fileList.update(fileName, result[i]);
		}
		//TODO
		if(!fileUtils.fileExists(fileName))
		{
			//if(fileList.numOwners(fileName) < 2)
			//{
				getFile(fileName, fileName);
			//}
		}
		break;
	}
	case 6: // DELETE FILE
		{
			std::string fileName = result[2];
			fileList.removeFile(fileName);
			fileUtils.removeFile(fileName);
			std::cout << "Deleted file: " << fileName << std::endl;
			log << "Deleted file: " << fileName << std::endl;
			break;
		}
	case 7: // Juice request (received at master)
	{
		std::cout << "Juice request received at Master process." << std::endl;
		
		boost::thread t = boost::thread(
		boost::bind(&Server::processJuiceMaster, this, result));
		break;
	}
	case 8: 
	{

		break;	
	}
	default:
		break;
	}
	startReceive();
}

void Server::ping(const std::string& ip, const std::string& message)
{
	try
	{
		boost::asio::io_service io_service;

		udp::resolver resolver(io_service);
		udp::resolver::query query(udp::v4(), ip, "55555");
		udp::endpoint receiver_endpoint = *resolver.resolve(query);

		udp::socket socket(io_service);
		socket.open(udp::v4());

		socket.send_to(boost::asio::buffer(message), receiver_endpoint);

	} catch (std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}

}

void Server::stop()
{
	running = false;
}

void Server::addToMembershipList(MembershipList::tuple& tuple)
{
	membershipList.add(tuple);
	std::string s = membershipList.tupleToString(tuple);
	log << s << " added to membership list." << std::endl;
}

void Server::listCaseHelper(std::string& message)
{
// Parse message and construct tuples.
// Add tuples to the membership list.
	std::vector<std::string> results1;
//boost::split(results, message, "__(", boost::token_compress_on);
	boost::algorithm::split_regex(results1, message, boost::regex("[(]"));
	for (int i = 1; i < results1.size(); ++i)
	{
		std::vector<std::string> results2;
		boost::split(results2, results1[i], boost::is_any_of(", "),
				boost::token_compress_on);
		std::string ip = results2[0];
		time_t timestamp = boost::lexical_cast<time_t>(results2[1]);
		int port = 55555;
		MembershipList::tuple tuple(ip, timestamp, port);
		membershipList.add(tuple);
		std::string s = membershipList.tupleToString(tuple);
		log << s << " added to membership list." << std::endl;
	}
}

/*
 * Ping the members of the membership list
 * every once in a while to make sure they're still
 * alive. Remove members that crash or don't respond.
 *
 */
void Server::pingServers()
{
	time_t time = std::time(NULL);
	std::vector<MembershipList::tuple> list = membershipList.get();
	while (true)
	{
		// Ping every second
		time_t diff = std::time(NULL) - time;
		if (diff > 1)
		{
			list = membershipList.get();
			time = std::time(NULL);
		}
		if (membershipList.size() > 1 && diff > 1)
		{
			// Ping the members to see if still alive
			std::string message = messageCreator.message(0, myIp, myPort);
			for (std::vector<MembershipList::tuple>::iterator it = list.begin();
					it != list.end(); ++it)
			{
				std::string ip = it->get<0>();
				if (ip.compare(myIp) != 0)
				{
					ping(ip, message);
				}
			}
		}
	}
}

/*
 * Checks the membership list for failed processes.
 * Also sets the Master's IP addres.
 */
void Server::checkForLeaves()
{
	std::vector<MembershipList::tuple> list;
	time_t time = std::time(NULL);
	while (true)
	{
		time_t diff = std::time(NULL) - time;
		if (diff >= 3) // allow for the 2 second ping-ack round-trip time
		{
			list = membershipList.get();

			for (std::vector<MembershipList::tuple>::iterator it = list.begin();
					it != list.end(); ++it)
			{
				std::string ip = it->get<0>();
				if (ip.compare(myIp) != 0)
				{
					time_t mbrTime = it->get<1>();
					if ((time - mbrTime) > 3)
					{
						std::string s = membershipList.tupleToString(*it);
						membershipList.remove(ip);
						std::cout << "" << std::endl;
						fileList.removeIp(ip);
						std::cout << s
								<< " timed out. Dropped from membership list."
								<< std::endl;
						log << s << " timed out. Dropped from membership list."
								<< std::endl;
						log << "Removing process " + ip + " from file list." << std::endl;
					}
				}
			}
			time = std::time(NULL);
		}
	}
}

void Server::setIntroducerIp(const std::string& ip)
{
	introducerIp = ip;
}

std::string Server::getIntroducerIp()
{
	return introducerIp;
}

void Server::getFile(const std::string& fileName, const std::string& sdfsFileName)
{
	// Check if file is on local machine
	if (fileUtils.fileExists(fileName))
	{
		std::cout << "File already in local dir!" << std::endl;
		return;
	}
	if (!fileList.isInList(sdfsFileName))
	{
		std::cout << "File not on the network!" << std::endl;
		return;
	}
	// If not, then send query another process for file
	std::string tmp = myIp + " " + sdfsFileName;
	std::string message = "0 " + tmp;
	std::string ipWithFile = fileList.getIp(sdfsFileName);

	tcpClient.query(ipWithFile, message, fileName);

	// Notify Master of get
	std::string msg = "5_" + myIp + "_" + sdfsFileName + "_" + myIp + "_" + myIp;
	log << sdfsFileName << " added to file list." << std::endl;
	ping(membershipList.masterIp(), msg);
}

void Server::putFile(const std::string& fileName, const std::string& sdfsFileName)
{
	// Check if file already on the file list
	if (fileList.isInList(sdfsFileName))
	{
		//std::cout << "File already on the network!" << std::endl;
		return;
	}
	else if (!fileUtils.fileExists(fileName))
	{
		std::cout << "File: " << fileName << " doesn't exist!" << std::endl;
		return;
	}
	// If not, then tell master process to add it!
	std::string message = "4_" + myIp + "_" + fileName + "_" + sdfsFileName;
	ping(myIp, message); // UDP
	ping(membershipList.masterIp(), message);
}

void Server::deleteFile(const std::string& sdfsFileName)
{
	// Check if file is on the file list
	if (!fileList.isInList(sdfsFileName))
	{
		std::cout << "File doesn't exist!" << std::endl;
		return;
	}

	// If not, then send command to processes to remove the file
	std::string msg = "6_" + myIp + "_" + sdfsFileName;
	std::vector<MembershipList::tuple> list;
	list = membershipList.get();
	for (std::vector<MembershipList::tuple>::iterator it = list.begin(); it != list.end(); ++it)
	{
		std::string ip = it->get<0>();
		ping(ip, msg);
	}
}

void Server::maple(std::vector<std::string>& args)
{
	// check if file(s) on sdfs
	for(int i = 3; i < args.size(); i++)
	{
		std::string filename = "/tmp/" + args[i];
		if(!fileList.isInList(filename))
		{
			std::cout << "sdfs_file: " << args[i] << " doesn't exist!" << std::endl;
			std::cout << "Please try again." << std::endl;
			return;
		}
	}

	// check if maple_exe exists
	if(!fileUtils.fileExists(args[1]))
	{
		std::cout << "maple_exe: " << args[1] << " doesn't exist!" << std::endl;
		std::cout << "Please try again." << std::endl;
		return;
	}
	log << "Running maple request." << std::endl;
	// run maple_exe for each input file
	for(int i = 3; i < args.size(); i++)
	{
		std::string cmd = "./" + args[1] + " /tmp/" + args[i] + " " + args[2];
		system(cmd.c_str()); //TODO: threadbundle
	}

	// put files onto sdfs
	std::string keyfilename = "/tmp/";
	keyfilename += args[2];
	keyfilename += "keys";
	std::ifstream input(keyfilename.c_str());
	std::string token;
	while(input >> token)
	{	
		std::string filename = "/tmp/";
		filename += args[2];
		filename += token;
		boost::this_thread::sleep(boost::posix_time::milliseconds(1000));
		putFile(filename, filename);
	}
	putFile(keyfilename, keyfilename);
	std::cout << "\nMaple complete." << std::endl;
}


void Server::juice(std::vector<std::string>& args)
{
	// check if juice_exe exists
	if(!fileUtils.fileExists(args[1]))
	{
		std::cout << "juice_exe: " << args[1] << " doesn't exist!" << std::endl;
		std::cout << "Please try again." << std::endl;
		return;
	}
	// Check if intermediate file name with keys exists
	std::string sdfs_inter_filename_prefix = "/tmp/";
	sdfs_inter_filename_prefix += args[3];
	std::string sdfs_inter_filename_prefix_keys =  "/tmp/" + args[3] + "keys";
	std::string sdfs_dest_file = "/tmp/";
	sdfs_dest_file += args[4];
	
	if(!fileList.isInList(sdfs_inter_filename_prefix_keys))
	{
		std::cout << "intermediate file(s) with prefix: " << args[3] << " don't exist!" << std::endl;
		std::cout << "Please try again." << std::endl;
		return;
	}

	// tell master that we would like to issue a juice request
	std::string message = "7_" + myIp + "_";
	message += args[2];
	message += "_" + sdfs_inter_filename_prefix + "_" + sdfs_dest_file + "_";
	message += args[1];
	ping(membershipList.masterIp(), message);
}

void Server::pingFileList()
{
	while(true)
	{
		boost::this_thread::sleep(boost::posix_time::milliseconds(5000));
		if(myIp == membershipList.masterIp())
		{
			std::vector<FileList::tuple> fl = fileList.get();
			for(int i = 0; i < fl.size(); ++i)
			{
					std::string fileName = fl[i].get<0>();
					std::string message = "5_" + myIp + "_" + fileName + "_";
					for(int j = 0; j < fl[i].get<1>().size(); ++j)
					{
							message += "_";
							if(fl[i].get<1>()[j] != "") // Removed IP
								message += fl[i].get<1>()[j];
					}
					boost::this_thread::sleep(boost::posix_time::milliseconds(1000));
					std::vector<MembershipList::tuple> list;
					list = membershipList.get();

					// Send list to each of the members except ourself
					for (std::vector<MembershipList::tuple>::iterator it = list.begin(); it != list.end(); ++it)
					{
						std::string ip = it->get<0>();
						if (ip.compare(myIp) != 0)
						{
							ping(ip, message);
						}
					}
			}
		}
	}
}

// Divide Juice task according to # of desired tasks
// and assign juice tasks to processes.
// If they don't complete within a certain amount of time,
// then reassign the task to a different process.
// msg_fmt_idx: 0:id, 1:ip, 2:num, 3:interfile, 4:destfile 5:exe
void Server::processJuiceMaster(std::vector<std::string> args)
{
	log << "Running juice request on " << args[2] << " processes." << std::endl;
	
	// get keys that must be processed
	std::string sdfskeyfile = args[3];
	sdfskeyfile += "keys";
	std::ifstream is(sdfskeyfile.c_str());
	std::vector<std::string> keys;
	std::string token;
	while(is >> token)
	{
		keys.push_back(token);
	}
	std::set<std::string> set(keys.begin(), keys.end());
	std::string interfile = args[3];
	std::string destfile = args[4];
	std::vector<std::string> fileNames;
	std::set<std::string>::iterator it;
	for(it = set.begin(); it != set.end(); it++)
	{
		std::string tmpinter = interfile + *it;
		std::string tmpdest = destfile + *it;
		std::string cmd = "./";
		cmd += args[5];
		cmd += " ";
		cmd += tmpinter;
		cmd += " ";
		cmd += tmpdest;
		system(cmd.c_str());
		boost::this_thread::sleep(boost::posix_time::milliseconds(1000));
		putFile(tmpdest, tmpdest);
		fileNames.push_back(tmpdest);
	}
	// Sum all keys 
	std::ofstream dest(destfile.c_str());	
	std::string line;
	std::map<std::string, int> keycount;
	for(int i = 0; i < fileNames.size(); i++)
	{
		std::ifstream input(fileNames[i].c_str());
		std::getline(input, line);
		std::vector<std::string> result;
		boost::split(result, line, boost::is_any_of(" "));
		keycount[result[1]] += boost::lexical_cast<int>(result[3]);
	}

	// write to dest file
  std::map<std::string, int>::const_iterator itr;
  for(itr = keycount.begin(); itr != keycount.end(); ++itr) 
      dest << "( " << (*itr).first << " , " << (*itr).second << " )" << std::endl;
	putFile(destfile, destfile);
	std::cout << "\nJuice complete." << std::endl;
}

void Server::processJuice(std::vector<std::string> args)
{
}
