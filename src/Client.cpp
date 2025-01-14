/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: akuburas <akuburas@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/08 09:49:48 by akuburas          #+#    #+#             */
/*   Updated: 2025/01/13 11:30:23 by akuburas         ###   ########.fr       */
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

#include "../inc/Client.hpp"

Client::Client( int fd, const sockaddr_in& addr )
{
	this->_clientFd = fd;
	this->_clientAddr = addr;
}

Client::~Client()
{
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


int	Client::getClientFd() const
{
	return (this->_clientFd);
}

struct pollfd *Client::getFds()
{
	return (this->_fds);
}

void	Client::setPollFd( struct pollfd *fds )
{
	this->_fds = fds;
}

void	Client::setNick( std::string nick )
{
	this->_nick = nick;
}

std::string	Client::getNick()
{
	return (this->_nick);
}

void	Client::setUser( std::string user )
{
	this->_user = user;
}

std::string	Client::getUser()
{
	return (this->_user);
}

void	Client::setRealname( std::string realname )
{
	this->_realname = realname;
}

std::string	Client::getRealname()
{
	return (this->_realname);
}

bool	Client::hasMode(char mode) const
{
	return (_userModes.find(mode) != _userModes.end());
}

void	Client::addMode(char mode)
{
	_userModes.insert(mode);
}

void	Client::removeMode(char mode)
{
	_userModes.erase(mode);
}

std::string Client::getModes() const
{
	std::string modes;
	for (char mode : _userModes)
	{
		modes += mode;
	}
	return (modes);
}
