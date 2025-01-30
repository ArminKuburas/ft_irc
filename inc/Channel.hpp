/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fdessoy- <fdessoy-@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/18 11:18:19 by akuburas          #+#    #+#             */
/*   Updated: 2025/01/29 09:38:46 by fdessoy-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <set>
#include "Client.hpp"

class Client;

class Channel
{
	private:
		std::string				_name;
		std::string				_topic;
		std::set<Client*>		_members;
		std::set<Client*>		_operators;
		bool 					_isPrivate;
		bool					_isInviteOnly;
		void 					setName( const std::string& name );
	public:
		Channel(const std::string &name, const std::string &topic, bool isPrivate, bool isInviteOnly);
		~Channel();

		// Getters
		const std::string 		getName() const;
		const std::string 		getTopic() const;
		const std::set<Client*> getMembers() const;
		const std::set<Client*> getOperators() const;
		const bool&				getIsPrivate() const;
		const bool&				getIsInviteOnly() const;

		// Setters
		void					setTopic(const std::string& newTopic);
		void					setPrivate(bool isPrivate);
		void					setInviteOnly(bool isInviteOnly);

		// Membership management
		bool					addOperator(Client* client);
		void					addMember(Client* client);
		bool					removeMember(Client* client);
		bool					removeOperator(Client* client);
		
		// Utility
		bool 					isMember(Client* client) const;
		bool 					isOperator(Client* client) const;
		bool					isChannelEmpty() const;

};