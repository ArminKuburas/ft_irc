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
	uint64_t largeNumber = UINT64_C(18446744073709551615); // UINT64_C ensures portability for other architectures
	setMaxMembers(largeNumber);
	setName(name);
	setKey(key);
	if (!(topic == ""))
		setTopic(topic, "none");
	setPrivate(IsPrivate);
	setInviteOnly(isInviteOnly);
	_topic = topic;
	_hasMemberLimit = false;
}

Channel::~Channel()
{
}

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

bool Channel::getTopicFlag() const
{
	return (_operatorSetsTopic);
}

uint64_t	Channel::getNumberMaxMembers() const
{
	return (_maxMembers);
}

bool	Channel::getMaxMembers() const
{
	return (_hasMemberLimit);
}

uint64_t Channel::getNbMembers() const
{
	return static_cast<uint64_t>(_members.size());
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

void Channel::setTopic( const std::string& newTopic, const std::string& setter)
{
	_topic = newTopic;
	_topicSetBy = setter;
	_topicSetAt = time(NULL);
}

std::string	Channel::getSetter() const
{
	return (_topicSetBy);
}

time_t Channel::getTopicTime() const
{
	return (_topicSetAt);
}

void Channel::setPrivate( bool isPrivate )
{
	_isPrivate = isPrivate;
}

void Channel::setInviteOnly( bool isInviteOnly )
{
	_isInviteOnly = isInviteOnly;
}

void Channel::setTopicFlag( bool operatorSetsTopic )
{
	_operatorSetsTopic = operatorSetsTopic;
}

void Channel::limitMaxMembers( uint64_t limit )
{
	_maxMembers = limit;
}

void Channel::setMaxMembers( bool active )
{
	_hasMemberLimit = active;
}

void Channel::setModes(char mode)
{
	this->_channelModes.insert(mode);
	if (mode == 'i')
		this->setInviteOnly(true);
	else if (mode == 't')
		this->setTopicFlag(true);
	else if (mode == 'l')
		this->setMaxMembers(true);
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
		if (!this->isOperator(channelOperator) || !this->isMember(channelOperator)
			|| !this->isMember(target) || this->isOperator(target))
		{
			return (false);
		}
	}
	_operators.emplace(target);
	return (true);
}

bool	Channel::removeOperator(Client* channelOperator, Client* target)
{
	if (target == nullptr)
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
	if (mode == 'i')
		this->setInviteOnly(false);
	else if (mode == 't')
		this->setTopicFlag(false);
	else if (mode == 'l')
		this->setMaxMembers(false);
}

Client*	Channel::retrieveClient(std::string username)
{
	for (Client *member : _members)
	{
		if (member->getNick() == username)
			return (member);
	}
	return (nullptr);
}
