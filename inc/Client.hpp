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
#include <set>

class Client
{
	private:
		struct sockaddr_in	_clientAddr;
		int 				_clientFd;
		socklen_t			_clientAddrLen;
		struct pollfd		*_fds;
		std::string			_nick;
		std::string			_user;
		std::string			_realname;
		std::set<char>		_userModes;
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
		std::string	getNick();
		std::string	getUser();
		std::string	getRealname();

		// setters
		void	setClientAddr( sockaddr_in clientAddr );
		void	setClientAddrLen( socklen_t clientAddrLen );
		void	setClientFd( int clientFd );
		void	setPollFd( struct pollfd *fds );
		void	setNick( std::string nick );
		void	setUser( std::string user );
		void	setRealname( std::string realname );

		//mode
		bool	hasMode(char mode) const;
		void	addMode(char mode);
		void	removeMode(char mode);
		std::string getModes() const;
};