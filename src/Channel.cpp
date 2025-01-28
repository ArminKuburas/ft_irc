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

// Channel::Channel( const Channel& ref )
// {
// 	setName(ref._name);
// 	setTopic(ref._topic);
// 	setPrivate(ref._IsPrivate);
// 	setInviteOnly(ref._isInviteOnly);
// }

// Channel &Channel::operator=( const Channel& ref )
// {
// 	if (*this == ref)
// 		return (*this);
// 	setName(ref._name);
// 	setTopic(ref._topic);
// 	setPrivate(ref._isPrivate);
// 	setInviteOnly(ref._isInviteOnly);
// 	return (*this);
// }

// Getters
const std::string Channel::getName() const
{
	return (_name);
}

const std::string Channel::getTopic() const
{
	return (_topic);
}

const std::set<Client*> Channel::getMembers() const
{
	return (_members);
}

const std::set<Client*> Channel::getOperators() const
{
	return (_operators);
}

const bool& Channel::getIsPrivate() const
{
	return (_isPrivate);
}

const bool& Channel::getIsInviteOnly() const
{
	return (_isInviteOnly);
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
	_isPrivate = isPrivate;
}

void Channel::setInviteOnly( bool isInviteOnly )
{
	_isInviteOnly = isInviteOnly;
}

// Membership management
void Channel::addMember(Client* client)
{
	if (_members.empty())
		addOperator(client);
	_members.emplace(client);
}

bool Channel::removeMember(Client* client)
{
	auto it = _members.find(client);
	if (it != _members.end())
	{
		_members.erase(it);
		return (true);
	}
	return (false);
}

bool Channel::addOperator(Client* client)
{
	for (auto it = _operators.begin(); it != _operators.end(); ++it)
	{
		Client* existingClient = *it;

		if (existingClient->getUser() == client->getUser())
		{
			return (true);
		}
	}
	_operators.emplace(client);
	return (false);
}

bool Channel::removeOperator(Client* client)
{
	auto it = _operators.find(client);
	if (it != _operators.end())
	{
		_operators.erase(it);
		return (true);
	}
	return (false);
}

// Channel settings


// Utility
bool Channel::isMember(Client* client) const
{
	for (auto it = _members.begin(); it != _members.end(); ++it)
	{
		Client* possibleMember = *it;

		if ((possibleMember->getClientFd() == client->getClientFd()))
			return (true);
	}
	return (false);
}

bool Channel::isOperator(Client* client) const
{
	for (auto it = _operators.begin(); it != _operators.end(); ++it)
	{
		Client* possibleOperator = *it;
		
		if (possibleOperator->getClientFd() == client->getClientFd())
			return (true);
	}
	return (false);
}

bool Channel::isChannelEmpty() const
{
	if (_members.empty())
		return (true);
	return (false);
}