/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pmarkaid <pmarkaid@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/04 11:27:53 by akuburas          #+#    #+#             */
/*   Updated: 2025/02/10 13:37:37 by pmarkaid         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

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
#include <stdexcept>
#include <cstring>
#include <fcntl.h>
#include <poll.h>
#include <vector>
#include "Client.hpp"
#include <algorithm>
#include <functional>
#include <map>
#include <sstream>
#include <ctime>

#include "Channel.hpp"

#define SERVER_NAME "Zorg"
#define JUST_JOINED 1
#define UNIVERSAL_MSG 2
#define NORMAL_MSG 3

class Client;
class Channel;

using CommandHelper = std::function<void(Client&, const std::string&)>;

class Server
{
	private:
		static const int MAX_CLIENTS = 1024;
		int										_port;
		std::string								_name = SERVER_NAME;
		std::string								_password;
		int										_serverSocket;
		struct sockaddr_in						_serverAddr;
		std::vector<struct pollfd>				_poll_fds;
		std::vector<Client>						_clients;
		std::map<std::string, Channel> 			_channels;
		std::map<std::string, CommandHelper>	_commands;
		void disconnectClient(Client& client, const std::string& reason);
		void cleanupFd(int fd_index);
		void Help(Client& client, const std::string& message);
		bool handleNewConnection();
		bool handleClientData(size_t index);
		
	public:
		// constructor
		Server(int port, std::string password);
		
		// destructor
		~Server();

		// getters
		int									getPort();
		int									getSocket();
		const sockaddr_in&					getServerAddr() const;
		// pollfd		*getFdPoll();

		// setters
		void								setSocket( int socket );
		void								setServerAddr();

		// public methods
		void								portConversion( std::string port );
		void								Run();
		void								AddClient( int clientFd, sockaddr_in clientAddr, socklen_t clientAddrLen );
		void								BroadcastMessage(std:: string &messasge);
		void								SendToClient(Client& client, const std::string& message);
		void								SendToChannel(const std::string& channelName, const std::string& message, Client* sender, int code);
		void								handleMessage(Client& client, const std::string& message);
		int									connectionHandshake(Client& client, std::vector<std::string> messages, int fd);
		void								ModeHelperChannel(Client &client, std::map<std::string, Channel>::iterator it, char mode, bool adding, std::string code);
		bool 								MultipleChannels( const std::string& message );
		std::map<std::string, std::string>	MapChannels(const std::string& message );

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
		int  Pass(Client& client, const std::string& message);
		void Stats(Client& client, const std::string& message);
		void Whois(Client& client, const std::string& message);
    	void Part(Client& client, const std::string& message);
		void Topic(Client& client, const std::string& message);
		void Invite(Client& client, const std::string& message);
		void Kick(Client& client, const std::string& message);
		void Who(Client& client, const std::string& message);
		void List(Client& client, const std::string& message);


		void 								initializeCommandHandlers();
		std::vector<std::string>			splitMessages(const std::string& message);
		void 								checkClientTimeouts();


};