/* ****************************************************************************/
/*  ROFL:ROFL:ROFL:ROFL 													  */
/*          _^___      										 				  */
/* L     __/   [] \    										 			      */
/* LOL===__        \   			MY ROFLCOPTER GOES BRRRRRR				  	  */
/* L      \________]  					by fdessoy-				  			  */
/*         I   I     			(fdessoy-@student.hive.fi)				  	  */
/*        --------/   										  				  */
/* ****************************************************************************/

#include "../inc/Client.hpp"

Client::Client( std::string password )
{
	if (!password.empty())
		this->_password = password;
}

Client::~Client()
{
}

std::string Client::getPassword()
{
	return (this->_password);
}

sockaddr_in Client::getClientAddr()
{
	return (this->_clientAddr);
}

socklen_t 	Client::getClientAddrLen()
{
	return (this->_clientAddrLen);
}

void Client::setClientAddr( sockaddr_in clientAddr )
{
	this->_clientAddr = clientAddr;
}

void Client::setClientAddrLen( socklen_t clientAddrLen )
{
	this->_clientAddrLen = clientAddrLen;
}

void	Client::setClientFd( int clientFd )
{
	this->_clientFd = clientFd;
}
