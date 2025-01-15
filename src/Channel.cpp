/* ****************************************************************************/
/*  ROFL:ROFL:ROFL:ROFL 													  */
/*          _^___      										 				  */
/* L     __/   [] \    										 			      */
/* LOL===__        \   			MY ROFLCOPTER GOES BRRRRRR				  	  */
/* L      \________]  					by fdessoy-				  			  */
/*         I   I     			(fdessoy-@student.hive.fi)				  	  */
/*        --------/   										  				  */
/* ****************************************************************************/

#include "../inc/Channel.hpp"

Channel::Channel(const std::string &name, const std::string &topic, bool IsPrivate, bool isInviteOnly )
{
	setName(name);
	setTopic(topic);
	setPrivate(IsPrivate);
	setInviteOnly(isInviteOnly);
}

Channel::~Channel()
{
	
}

Channel::Channel( const Channel& ref )
{
	
}

Channel &Channel::operator=( const Channel& ref )
{

}

// Getters
const std::string& Channel::getName() const
{
	return (this->_name);
}

const std::string& Channel::getTopic() const
{
	return (this->_topic);
}

const std::set<Client*>& Channel::getMembers() const
{
	return (this->_members);
}

const std::set<Client*>& Channel::getOperators() const
{
	return (this->_operators);
}

const bool& getIsPrivate() const
{
	return (this->_isPrivate);
}

const bool& getIsInviteOnly() const
{
	return (this->_isInviteOnly);
}

// Setters
void Channel::setName( const std::string& name )
{
	_name = name;
}

void Channel::setTopic( const std::string& newTopic )
{
	_topic = newTopic;
}

void Channel::setPrivate( bool isPrivate )
{
	if (getIsPrivate() == false)
	{
		// need to message the client that it was successful and it is now private
		_isPrivate = true;
	}
	else // switch back
	{
		_isPrivate = false
	}
}

void Channel::setInviteOnly( bool isInviteOnly )
{
	if (getIsInviteOnly() == false)
	{
		// need to message the client that it was sucessful and it is now private
		_isInviteOnly = true;
	}
	else // switch back
	{
		_isInviteOnly = false;
	}
}

// Membership management
bool Channel::addMember(Client* client)
{
	// if the member is the first to enter the server, we shall call addOperator
	if (_members.empty())
	{
		addOperator(client);
		_members.emplace(client);
	}
	else
	{
		_members.emplace(client);
	}
}

bool Channel::removeMember(Client* client)
{
	for (auto it = _members.begin(); it != _members.end(); ++it)
	{
		if ((it->getNick() == client->getNick() || it->getUser() == client->getNick()) && (it->getClientFd() == client->getClientFd()))
		{
			_members.erase(client);
			return (true);
		}
	}
	return (false);
}

bool Channel::addOperator(Client* client)
{
	_operators.emplace(client);
}

bool removeOperator(Client* client)
{
	for (auto it = _operators.begin(); it != _operators.end(); ++it)
	{
		if ((it->getNick() == client->getNick() || it->getUser() == client->getNick()) && (it->getClientFd() == client->getClientFd()))
		{
			_operators.erase(client);
			return (true);
		}
	}
	return (false);
}

// Channel settings


// Utility
bool Channel::isMember(Client* client) const
{
	for (auto it = _members.begin(); it != _members.end(); ++it)
	{
		if ((it->getNick() == client->getNick() || it->getUser() == client->getNick()) && (it->getClientFd() == client->getClientFd()))
			return (true);
	}
	return (false);
}

bool Channel::isOperator(Client* client) const
{
	for (auto it = _operators.begin(); it != _operators.end(); ++it)
	{
		if ((it->getNick() == client->getNick() || it->getUser() == client->getNick()) && (it->getClientFd() == client->getClientFd()))
			return (true);
	}
	return (false);
}