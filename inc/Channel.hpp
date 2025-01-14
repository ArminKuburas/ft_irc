/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fdessoy- <fdessoy-@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/18 11:18:19 by akuburas          #+#    #+#             */
/*   Updated: 2025/01/14 15:34:49 by fdessoy-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <set>
#include "Client.hpp"


class Channel
{
	private:
		std::string				_name;
		std::string				_topic;
		std::set<Client>		_members;
		std::set<Client>		_operators;
		bool 					_isPrivate;
		bool					_isInviteOnly;
	public:
		Channel(const std::string &name, const std::string &topic, bool isPrivate, bool isInviteOnly);
		const std::string& getName() const;
		const std::string& getTopic() const;
		const std::set<Client*>& getMembers() const;
		const std::set<Client*>& getOperators() const;

		// Setters
		void setTopic(const std::string& newTopic);

		// Membership management
		bool addMember(Client* client);
		bool removeMember(Client* client);
		bool addOperator(Client* client);
		bool removeOperator(Client* client);

		// Channel settings
		void setPrivate(bool isPrivate);
		void setInviteOnly(bool isInviteOnly);

		// Utility
		bool isMember(Client* client) const;
		bool isOperator(Client* client) const;
};