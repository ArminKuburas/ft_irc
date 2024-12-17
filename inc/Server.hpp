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

class Server
{
	private:
		int					_port;
		int					_serverSocket;
		struct sockaddr_in	_serverAddr;
		std::vector<Client>	_clients;
		
	public:
		// constructor
		Server( std::string port );
		
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
};