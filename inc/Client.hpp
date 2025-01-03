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

#include "Server.hpp"

class Client
{
	private:
		struct sockaddr_in	_clientAddr;
		int 				_clientFd;
		socklen_t			_clientAddrLen;
		struct pollfd		*_fds;
	public:
		// constructor
		Client( int fd, const sockaddr_in& addr);
		
		// destructor
		~Client();
		
		// getters
		sockaddr_in getClientAddr();
		socklen_t 	getClientAddrLen();
		int 		getClientFd() const;
		struct pollfd *getFds();

		// setters
		void	setClientAddr( sockaddr_in clientAddr );
		void	setClientAddrLen( socklen_t clientAddrLen );
		void	setClientFd( int clientFd );
		void	setPollFd( struct pollfd *fds );
};