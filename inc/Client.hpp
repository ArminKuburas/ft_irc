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
#include <arpa/inet.h>

class Client
{
	private:
		struct sockaddr_in	_clientAddr;
		std::string			_clientHost;
		int 				_clientFd;
		socklen_t			_clientAddrLen;
		struct pollfd*		_fds;
		std::string			_nick;
		std::string			_user;
		std::string			_realName;
		std::set<char>		_userModes;
		bool				_isAuthenticated = false;
		bool				_isRegistered = false;
		time_t				_lastActivity = time(nullptr);;
		bool				_awaitingPong = false;
		time_t				_pingTime = 0;
		std::string			_messageBuffer;

	public:
		// constructor
		Client( int fd, const sockaddr_in& addr);
		
		// destructor
		~Client();
		
		// getters
		sockaddr_in 		getClientAddr();
		const std::string	getHost() const;
		socklen_t 			getClientAddrLen();
		int 				getClientFd() const;
		struct pollfd*		getFds();
		const std::string	getNick() const;
		const std::string	getUser() const;
		const std::string	getRealname() const;
		bool				getAuthentication() const;
		bool				getRegistration() const;
		time_t				getLastActivity() const;
		bool				getAwaitingPong() const;
		time_t				getPingTime() const;

		// setters
		void				setClientAddr( sockaddr_in clientAddr );
		void				setClientAddrLen( socklen_t clientAddrLen );
		void				setClientFd( int clientFd );
		void				setPollFd( struct pollfd *fds );
		void				setNick( std::string nick );
		void				setUser( std::string user );
		void				setRealname( std::string realname );
		void				setAuthentication(bool auth);
		void				setRegistration(bool reg);
		void				setLastActivity();
		void				setPingStatus(bool awaiting);

		//mode
		bool				hasMode(char mode) const;
		void				addMode(char mode);
		void				removeMode(char mode);
		std::string 		getModes() const;

		//handle messages
		void				appendBuffer(const char* buffer, size_t length);
		bool				hasCompleteMessage() const;
		std::string			getAndClearBuffer();
};