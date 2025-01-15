/* ****************************************************************************/
/*  ROFL:ROFL:ROFL:ROFL 													  */
/*          _^___      										 				  */
/* L     __/   [] \    										 			      */
/* LOL===__        \   			MY ROFLCOPTER GOES BRRRRRR				  	  */
/* L      \________]  					by fdessoy-				  			  */
/*         I   I     			(fdessoy-@student.hive.fi)				  	  */
/*        --------/   										  				  */
/* ****************************************************************************/

#pragma once

#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <fcntl.h>
#include <poll.h>
#include <vector>
#include "Client.hpp"
#include <algorithm>
#include <functional>
#include <map>
#include <sstream>
#include "Channel.hpp"

class Client;
class Channel;

using CommandHelper = std::function<void(Client&, const std::string&)>;

using CommandHelper = std::function<void(Client&, const std::string&)>;

class Server
{
	private:
		int										_port;
		std::string								_password;
		int										_serverSocket;
		struct sockaddr_in						_serverAddr;
		std::vector<Client>						_clients;
		std::map<std::string, Channel> 			_channels;
		std::map<std::string, CommandHelper>	_commands;
		
	public:
		// constructor
		Server(int port, std::string password);
		
		// destructor
		~Server();

		// getters
		int			getPort();
		int			getSocket();
		const sockaddr_in	&getServerAddr() const;
		// pollfd		*getFdPoll();

		// setters
		void		setSocket( int socket );
		void		setServerAddr();


		// public methods
		void portConversion( std::string port );

		void Run();
		void AddClient( int clientFd, sockaddr_in clientAddr, socklen_t clientAddrLen );
		void BroadcastMessage(std:: string &messasge);
		void SendToClient(Client& client, const std::string& message);

		void handleMessage(Client& client, const std::string& message);

		// Command handlers
		void Ping(Client& client, const std::string& message);
		void Pong(Client& client, const std::string& message);
		void Cap(Client& client, const std::string& message);
		void Nick(Client& client, const std::string& message);
		void User(Client& client, const std::string& message);
		void Mode(Client& client, const std::string& message);
		void Join(Client& client, const std::string& message);
		void Quit(Client& client, const std::string& message);
		void Priv(Client& client, const std::string& message);

		void initializeCommandHandlers();
		std::vector<std::string> splitMessages(const std::string& message);

};