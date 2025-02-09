/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fdessoy- <fdessoy-@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/18 11:18:19 by akuburas          #+#    #+#             */
/*   Updated: 2025/02/07 12:17:17 by fdessoy-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <set>
#include <cstdint>
#include "Client.hpp"

class Client;

class Channel
{
	private:
		std::string				_name;
		std::string				_topic;
		std::string				_topicSetBy;
		time_t					_topicSetAt;
		std::set<Client*>		_members;
		std::set<Client*>		_operators;
		std::string				_key;
		std::set<char>			_channelModes;
		bool 					_isPrivate;
		uint64_t				_maxMembers;
		bool					_hasMemberLimit;
		bool					_isInviteOnly;
		bool					_operatorSetsTopic;
		void 					setName( const std::string& name );
	public:
		Channel(const std::string &name, const std::string &key, const std::string &topic, bool isPrivate, bool isInviteOnly);
		~Channel();

		// Getters
		const std::string 		getName() const;
		const std::string		getKey() const;
		const std::string 		getTopic() const;
		const std::set<Client*> getMembers() const;
		const std::set<Client*> getOperators() const;
		const bool&				getIsPrivate() const;
		const bool&				getIsInviteOnly() const;
		std::string				getModes() const;
		bool					getTopicFlag() const;
		std::string				getSetter() const;
		time_t					getTopicTime() const;

		// Setters
		void					setTopic(const std::string& newTopic, const std::string& setter);
		uint64_t				getNumberMaxMembers() const;
		bool					getMaxMembers() const;
		uint64_t				getNbMembers() const;

		// Setters
		void					setTopic(const std::string& newTopic);
		void					setKey(const std::string& key);
		void					setPrivate(bool isPrivate);
		void					setInviteOnly(bool isInviteOnly);
		void					setModes(char mode);
		void 					setTopicFlag( bool operatorSetsTopic );
		void 					setMaxMembers( bool active );

		// Membership management
		void					addMember(Client* client);
		bool					removeMember(Client* client);
		bool					addOperator(Client* channelOperator, Client* target);
		bool					removeOperator(Client* channelOperator, Client* target);
		bool					changeKey(Client* channelOperator, std::string newKey);
		void 					limitMaxMembers( uint64_t limit );
		
		// Utility
		bool 					isMember(Client* client) const;
		bool 					isOperator(Client* client) const;
		bool					isChannelEmpty() const;
		bool					noOperators() const;
		bool					hasMode(char mode) const;
		void					removeMode(char mode);
		Client*					retrieveClient( std::string username );
};