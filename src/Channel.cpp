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

Channel::Channel(const std::string &name, const std::string &key, const std::string &topic, bool IsPrivate, bool isInviteOnly )
{
	setName(name);
	setKey(key);
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

const std::string Channel::getKey() const
{
	return (_key);
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

std::string Channel::getModes() const
{
	std::string modes;
	for (char mode : this->_channelModes)
	{
		modes += mode;
	}
	return (modes);
}

// Setters
void Channel::setName( const std::string& name )
{
	_name = name;
}

void Channel::setKey( const std::string& key )
{
	_key = key;
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

void Channel::setModes(char mode)
{
	this->_channelModes.insert(mode);
}

// Membership management
void Channel::addMember(Client* client)
{
	if (_members.empty())
		_operators.emplace(client);
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

bool Channel::addOperator(Client* channelOperator, Client* target)
{
	if (!this->noOperators())
	{
		std::cout << "we got inside the no operator clause, which shouldn't happen" << std::endl;
		if (!this->isOperator(channelOperator) || !this->isMember(channelOperator)
			|| !this->isMember(target) || this->isOperator(target))
		{
			std::cout << "we got inside the clause " << std::endl;
			return (false);
		}
	}
	_operators.emplace(target);
	return (true);
}

bool	Channel::removeOperator(Client* channelOperator, Client* target, bool leaving)
{
	if (leaving)
	{
		if (!this->isOperator(channelOperator) || !this->isMember(channelOperator))
			return (false);
		_operators.erase(channelOperator);
		return (true);
	}
	if (!this->isOperator(channelOperator) || !this->isMember(channelOperator)
		|| !this->isMember(target) || !this->isOperator(target))
		return (false);
	_operators.erase(target);
	return (true);
}

bool	Channel::changeKey( Client* channelOperator, std::string newKey )
{
	if (!this->isMember(channelOperator) || !this->isOperator(channelOperator)
		|| newKey == this->_key)
		return (false);
	this->setKey(newKey);
	return (true);
}

// Utility
bool	Channel::isMember(Client* client) const
{
	for (auto it = _members.begin(); it != _members.end(); ++it)
	{
		Client* possibleMember = *it;

		if ((possibleMember->getClientFd() == client->getClientFd()))
			return (true);
	}
	return (false);
}

bool	Channel::isOperator(Client* client) const
{
	for (auto it = _operators.begin(); it != _operators.end(); ++it)
	{
		Client* possibleOperator = *it;
		
		if (possibleOperator->getClientFd() == client->getClientFd())
			return (true);
	}
	return (false);
}

bool	Channel::isChannelEmpty() const
{
	if (_members.empty())
		return (true);
	return (false);
}

bool	Channel::noOperators() const
{
	if (_operators.empty())
		return (true);
	return (false);
}

bool	Channel::hasMode(char mode) const
{
	return (this->_channelModes.find(mode) != this->_channelModes.end());
}

void	Channel::removeMode(char mode)
{
	_channelModes.erase(mode);
}

/** TO DO: 
 * 
 * 1. Manage to add operators after creation of the server (in mode +o the syntax is: /mode #channel +o <user>)
 * 2. Check for no operators left in the server and make the first of the list operator
 * 3. Add password functionality to server. Before and after server creation.
 * 
*/