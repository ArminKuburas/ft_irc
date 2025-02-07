/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: akuburas <akuburas@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/08 09:49:38 by akuburas          #+#    #+#             */
/*   Updated: 2025/02/07 11:00:21 by akuburas         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

/* ****************************************************************************/
/*  ROFL:ROFL:ROFL:ROFL 													  */
/*          _^___      										 				  */
/* L     __/   [] \    										 			      */
/* LOL===__        \   			MY ROFLCOPTER GOES BRRRRRR					  */
/* L      \________]  					by fdessoy-				  			  */
/*         I   I     			(fdessoy-@student.hive.fi)				  	  */
/*        --------/   										  				  */
/* ****************************************************************************/

#include "../inc/Server.hpp"

Server::Server(int port, std::string password)
{
	this->_port = port;
	this->_password = password;
	initializeCommandHandlers();
}

void Server::initializeCommandHandlers()
{
	_commands["CAP"] = [this](Client& client, const std::string& message)		{Cap(client, message); };
	_commands["NICK"] = [this](Client& client, const std::string& message) 		{Nick(client, message); };
	_commands["USER"] = [this](Client& client, const std::string& message) 		{User(client, message); };
	_commands["PING"] = [this](Client& client, const std::string& message) 		{Ping(client, message); };
	_commands["MODE"] = [this](Client& client, const std::string& message) 		{Mode(client, message); };
	_commands["PRIVMSG"] = [this](Client& client, const std::string& message) 	{Priv(client, message); };
	_commands["JOIN"] = [this](Client& client, const std::string& message)		{Join(client, message); };
	_commands["PART"] = [this](Client& client, const std::string& message)		{Part(client, message); };
	_commands["QUIT"] = [this](Client& client, const std::string& message) 		{Quit(client, message); };
	_commands["PASS"] = [this](Client& client, const std::string& message) 		{Pass(client, message); };
	_commands["STATS"] = [this](Client& client, const std::string& message) 	{Stats(client, message); };
	_commands["WHOIS"] = [this](Client& client, const std::string& message) 	{ Whois(client, message); };
	_commands["TOPIC"] = [this](Client& client, const std::string& message) 	{ Topic(client, message); };
	_commands["INVITE"] = [this](Client& client, const std::string& message) 	{ Invite(client, message); };
	_commands["KICK"] = [this](Client& client, const std::string& message) 		{ Kick(client, message); };
}

Server::~Server()
{
}

void Server::portConversion( std::string port )
{
	int port_number = std::stoi(port);
	if (port_number < 1 || port_number > 65535)
	{
		std::cout << "Error: port number must be in the range 1-65535" << std::endl;
		return ;
	}
	this->_port = port_number;
}

int	Server::getPort()
{
	return (this->_port);
}

int Server::getSocket()
{
	return (this->_serverSocket);
}
void	Server::setSocket( int socket )
{
	this->_serverSocket = socket;
}

const sockaddr_in	&Server::getServerAddr() const
{
	return (this->_serverAddr);
}

void	Server::setServerAddr()
{
	memset(&this->_serverAddr, 0, sizeof(this->_serverAddr));
	this->_serverAddr.sin_family = AF_INET;
	this->_serverAddr.sin_port = htons(this->getPort());
	this->_serverAddr.sin_addr.s_addr = INADDR_ANY;
}

void	Server::Run()
{
	struct pollfd fds[1024];
    int nfds = 1;

    fds[0].fd = this->_serverSocket;
    fds[0].events = POLLIN;

    while (true) 
	{
        int poll_result = poll(fds, nfds, -1);
        if (poll_result < 0) {
            perror("poll failed");
            return;
        }

        for (int i = 0; i < nfds; ++i) {
            if (fds[i].revents & POLLIN) {
                if (fds[i].fd == this->_serverSocket) {
					// Handling incoming connection requests on the listening socket
                    // New client connection
                    struct sockaddr_in client_addr;
                    socklen_t client_addr_len = sizeof(client_addr);

                    int client_fd = accept(this->_serverSocket, (struct sockaddr*)&client_addr, &client_addr_len);
                    if (client_fd < 0) {
                        perror("accept failed");
                        continue;
                    }
					// Make the new socket non-blocking
					int flags = fcntl(client_fd, F_GETFL, 0);
					fcntl(client_fd, F_SETFL, flags | O_NONBLOCK);

                    this->_clients.emplace_back(client_fd, client_addr);
					fds[nfds].fd = client_fd;
                    fds[nfds].events = POLLIN;

                    std::cout << "[" + this->_name +"] New client connected: " << client_fd << std::endl;
					++nfds;
                } else {
					// Handling data on established client connections
                    char buffer[1024];
                    ssize_t bytes_read = read(fds[i].fd, buffer, sizeof(buffer) - 1);
					//std::cout << "Number of bytes read: " << bytes_read << std::endl;
                    if (bytes_read <= 0) {
                        std::cout << "[Zorg] Disconnecting client: " << fds[i].fd << std::endl;
						for (auto& client : _clients)
						{
							if(client.getClientFd() == fds[i].fd){
								disconnectClient(client);
								break;
							}
						}
						fds[i] = fds[--nfds];
						--i;
					} else {
						buffer[bytes_read] = '\0';
						std::string receivedData(buffer);

						// Identify which client sent the message using the fd
						auto client = std::find_if(_clients.begin(), _clients.end(), [fd = fds[i].fd](const Client& client) { return client.getClientFd() == fd; });
						std::vector<std::string> messages = splitMessages(receivedData);
						if (!client->getRegistration()) {
							int grant_access = connectionHandshake(*client, messages); // Pass individual message
							if (!grant_access) {
								disconnectClient(*client);
								--i;
							}
						} else {
							for(const std::string& message : messages) {
								std::cout << client->getClientFd() << " >> " << message << std::endl;
								handleMessage(*client, message);
							}
						}
					}
				}
			}
		}
	}
}

int Server::connectionHandshake(Client& client, std::vector<std::string> messages) {

	std::cout << "[Zorg] Connection Handshake......." << std::endl;
	for(const std::string& message : messages) {
		std::istringstream stream(message);
		std::string command;
		stream >> command;
		std::transform(command.begin(), command.end(), command.begin(), ::toupper);
		
		std::cout << client.getClientFd() << " >> " << message << std::endl;

		if (command == "PASS") {
			int grant_access = Server::Pass(client, message);
			if (!grant_access) {
				SendToClient(client, ":" + _name + " 464 * :Password incorrect\r\n");
				return 0;
			}
			client.setAuthentication(true);
		} else if (command == "NICK") {
			// Only allow NICK/USER after PASS
			if (!client.getAuthentication()) {
				SendToClient(client, ":" + _name + " 451 * :You have not registered\r\n");
				return 0;
			}
			Server::Nick(client, message);
		} else if (command == "USER") {
			if (!client.getAuthentication()) {
				SendToClient(client, ":" + _name + " 451 * :You have not registered\r\n");
				return 0;
			}
			Server::User(client, message);
		} else if (command == "CAP") {
			Server::Cap(client, message);
		} else {
			SendToClient(client, ":" + _name + " 421 " + command + " :Unknown command\r\n");
		}
	}
	// Register user
	if(client.getAuthentication() && !client.getNick().empty() && !client.getUser().empty()){
		client.setRegistration(true);
		SendToClient(client, ":" + _name + " 001 " + client.getNick() + " :Welcome to the server\r\n");
	}
	return 1;
}

void Server::Stats(Client& client, const std::string& message){
	std::istringstream stream(message);
	std::string command, stat_option;
	stream >> command >> stat_option;

	if(stat_option.empty()){
		SendToClient(client, ":" + _name + " You need to provide a specific stats option [N]\r\n");
	}
	if(stat_option ==  "N"){
		for (const auto& clients : _clients)
		{
			SendToClient(client, ":" + _name + " STATS N " + std::to_string(clients.getClientFd()) + " :" + clients.getNick() + "\r\n");
		}
		SendToClient(client, ":" + _name + " 219 " + stat_option + " :End of /STATS report\r\n");
	}else
		SendToClient(client, ":" + _name + " Invalid stats option\r\n");
}

std::vector<std::string> Server::splitMessages(const std::string& message)
{
	std::vector<std::string> messages;
	std::istringstream stream(message);
	std::string line;
	
	while (std::getline(stream, line, '\n'))
	{
		if (!line.empty() && line.back() == '\r')
		{
			line.pop_back();
		}
		if (!line.empty())
		{
			messages.push_back(line);
		}	
	}
	return messages;
}

void Server::AddClient( int clientFd, sockaddr_in clientAddr, socklen_t clientAddrLen )
{
    this->_clients.emplace_back(clientFd, clientAddr);
    this->_clients.back().setClientAddrLen(clientAddrLen);
}

void Server::BroadcastMessage(std:: string &messasge)
{
	for (auto& client : _clients) {
		send(client.getClientFd(), messasge.c_str(), messasge.size(), 0);
	}
}

void Server::handleMessage(Client& client, const std::string& message)
{
	std::istringstream stream(message);
	std::string command;
	stream >> command;
	
	auto handler = _commands.find(command);
	if (handler != _commands.end()) 
	{
		handler->second(client, message);
	}
	else
	{
		SendToClient(client, ":" + this->_name + " 421 * " + command + " :Unknown command\r\n");
	}
}

void Server::SendToClient(Client& client, const std::string& message)
{
	if (client.getClientFd() == -1) {
		std::cerr << "[Zorg] Invalid file descriptor for client " << client.getClientFd() << std::endl;
	return;
	}
	ssize_t bytes_sent = send(client.getClientFd(), message.c_str(), message.length(), 0);
	if (bytes_sent < 0) {
		std::cerr << "[Zorg] Send failed. Error code: " << errno << " - " << strerror(errno) << std::endl;
	}
	else {
		std::cout << client.getClientFd() << " << " << message;
	}
	// check that the whole message was sent
	if (bytes_sent != static_cast<ssize_t>(message.size())) {
		std::cerr << "[Zorg] Warning: Not all bytes were sent to " << client.getClientFd() << std::endl;
	}
}

void Server::disconnectClient(Client& client) {
	SendToClient(client, "ERROR :Closing Link: " + client.getNick() + " (Client Quit)\r\n");
	shutdown(client.getClientFd(), SHUT_WR);
	close(client.getClientFd());
	_clients.erase(std::remove_if(_clients.begin(), _clients.end(),
		[fd = client.getClientFd()](const Client& c) { return c.getClientFd() == fd; }), _clients.end());
}

void Server::Cap(Client& client, const std::string& message)
{
	(void)message;
	SendToClient(client, ":" +this->_name + " CAP * LS :*\r\n");
}

void Server::Nick(Client& client, const std::string& message)
{
	std::string command, newNickname;
	std::istringstream stream(message);
	stream >> command >> newNickname;
	if (newNickname.empty()) {
		SendToClient(client, ":" +this->_name + " 431 " + client.getNick() + " :No nickname given\r\n");
		return;
	}
	// Check if the nickname is valid
	bool valid_nickname = true;
	if(newNickname.length() > 9)
		valid_nickname = false;
	// First character must be a letter, '_' or '|
	if (newNickname.empty() || !(isalpha(newNickname[0]) || newNickname[0] == '_' || newNickname[0] == '|')) {
		valid_nickname = false;
	}
	// Check valid characters
	for (char c : newNickname) {
		if (!(isalnum(c) || c == '-' || c == '_' || c == '|' ||
			  c == '[' || c == ']' || c == '\\' || c == '`' ||
			  c == '^' || c == '{' || c == '}')) {
			valid_nickname = false;
		}
	}
	if(!valid_nickname){
		SendToClient(client, ":" + _name + " 432 " + client.getNick() + " :Erroneous nickname\r\n");
		return;
	}

	// Check if nickname is already in use
	for (const auto& existingClient : _clients) {
		if (existingClient.getNick() == newNickname) {
				SendToClient(client, ":" + _name + " 433 * " + newNickname + " :Nickname is already in use\r\n");
				return;
			}
		}
	// All is good. Set nick
	std::string oldNickname = client.getNick();
	client.setNick(newNickname);
	SendToClient(client, ":" + oldNickname + "!" + client.getUser() + "@" + client.getHost() + " NICK :" + client.getNick() + "\r\n");
		
	// NOT IMPLEMENTED
	// ERR_NICKCOLLISION not implemented
			//-> same nickname in two servers
	// ERR_UNAVAILRESOURCE
			//-> The requested nickname or channel is temporarily unavailable (e.g., a user just quit and their nickname is still reserved).
	// ERR_RESTRICTED (484)
			// ->  The client is on a restricted connection and cannot change their nickname or perform certain actions.
}

void Server::User(Client& client, const std::string& message)
{
	if(!client.getUser().empty()){
		SendToClient(client, ":" + _name + " 462 "+ client.getNick() + " USER :You may not reregister\r\n");
		return;
	}

	std::istringstream stream(message);
	std::string command, username, hostname, servername, realname;

	stream >> command >> username >> hostname >> servername;
	getline(stream, realname);
	if(!realname.empty() && realname[0] == ' ')
		realname = realname.substr(2);
	if (username.empty())
	{
		SendToClient(client, ":" + _name + " 461 "+ client.getNick() + "USER :Not enough parameters\r\n");
		return;
	}
	client.setUser(username);
	client.setRealname(realname);
}

void Server::Whois(Client& client, const std::string& message)
{
	std::istringstream stream(message);
	std::string command, target;
	stream >> command >> target;

	if (target.empty()) {
		SendToClient(client, ":" + _name + " 431 " + client.getNick() + " :No nickname given\r\n");
		return;
	}

	// Find target client
	auto targetClient = std::find_if(_clients.begin(), _clients.end(),
		[&target](const Client& c) { return c.getNick() == target; });

	if (targetClient == _clients.end()) {
		SendToClient(client, ":" + _name + " 401 " + client.getNick() + " " + target + " :No such nick/channel\r\n");
		return;
	}

	// Send RPL_WHOISUSER (311)
	SendToClient(client, ":" + _name + " 311 " + client.getNick() + " " + targetClient->getNick() + " " + targetClient->getUser() + " " + targetClient->getHost() + " * :" + targetClient->getRealname() + "\r\n");

	// Send RPL_WHOISSERVER (312)
	SendToClient(client, ":" + _name + " 312 " + client.getNick() + " " + targetClient->getNick() + " " + _name + " :IRC Server\r\n");

	// Send RPL_ENDOFWHOIS (318)
	SendToClient(client, ":" + _name + " 318 " + client.getNick() + " " + targetClient->getNick() + " :End of /WHOIS list\r\n");
}

void Server::Ping(Client& client, const std::string& message)
{
	size_t pos = message.find(" ");
	if (pos == std::string::npos || pos + 1 >= message.size())
	{
		SendToClient(client, ":" + this->_name + " 409 " + "ERR_NOORIGIN :No origin specified\r\n");
		return;
	}
	std::string server1 = message.substr(pos + 1);
	if (server1.empty())
	{
		SendToClient(client, ":" + this->_name + " 409 " + "ERR_NOORIGIN :No origin specified\r\n");
		return;
	}
	//PONG response.
	SendToClient(client, "PONG " + server1 + "\r\n");
}

void Server::Mode(Client& client, const std::string& message)
{
	std::istringstream stream(message);
	std::string command, target, modeChanges, targetUser;
	stream >> command >> target >> modeChanges >> targetUser;
	bool channel = false;
	bool user = false;
	

	// check if target is either user or channel
	if (modeChanges.empty() || target.empty())
	{
		SendToClient(client, ":" + _name + " 221 " + "RPL_UMODEIS " + client.getModes() + "\r\n");
		return ;
	}
	if (target[0] == '#') // checking for channel
	{
		for (auto it = _channels.begin(); it != _channels.end(); ++it)
		{
			if (it->second.getName() == target)
				channel = true;
			else
			{
				SendToClient(client, ":" + _name + " 403 " + target +  ": Invalid channel name\r\n");
				return ;
			}
		}
	}
	else
	{
		if (target == client.getNick())
		{
			user = true;
		}
		else
		{
			// ERR_USERSDONTMATCH
			SendToClient(client, ":" + _name + " 502 " + ":Cannot change mode for another user\r\n");
			return ;
		}
	}
	// command: MODE
	// target: user or channel (need to differentiate)
	// modeChanges: flags +i +p +o +k
	if (user)
	{		
		bool adding = true;
		for (char ch : modeChanges)
		{
			if (ch == '+')
				adding = true;
			else if (ch == '-')
				adding = false;
			else if (ch == 'i') // this works with or without the addition sign?!
			{
				if (adding)
				{
					client.addMode(ch);
					SendToClient(client, ":" + this->_name + " 221 " + client.getNick() + " +i\r\n");
				}
				else
					client.removeMode(ch);
			}
			else
			{
				// ERR_UMODEUNKNOWNFLAG
				SendToClient(client, ":" + this->_name + " 501 " + ":Unknown mode flag\r\n");
			}
		}
	}
	else if (channel)
	{
		auto it = _channels.find(target);
		if (it == _channels.end())
		{
			// ERR_BADCHANMASK
			SendToClient(client, ":" + _name + " 476 " + client.getNick() + " " + target + ": invalid channel name" + "\r\n");
			return ;
		}
		bool adding = true;
		for (char ch : modeChanges)
		{
			std::string operatorPrivilege = " 324 ";
			std::string noOperatorPrivilege = " 482 ";
			if (ch == '+')
				adding = true;
			else if (ch == '-')
				adding = false;
			else if (ch == 'i')
			{
				if (it->second.isOperator(&client) && it->second.isMember(&client))
					ModeHelperChannel(client, it, ch, adding, operatorPrivilege);
				else
					SendToClient(client, ":" + this->_name + noOperatorPrivilege + client.getNick() + " " + target + ":you don’t have operator privileges to change modes\r\n");
			}
			else if (ch == 'k') // insert key to channel
			{
				if (it->second.isOperator(&client) && it->second.isMember(&client)) // pre-existing key needs a different treament for error
				{
					if (!it->second.getKey().empty())
					{
						SendToClient(client, ":" + this->_name + " 467 " + client.getNick() + " " + target + ":key already set\r\n");
						return ;
					}
					if (targetUser.empty())
					{
						SendToClient(client, ":" + this->_name + " 461 " + client.getNick() + " " + target + ":key already set\r\n");
						return ;
					}
					if (adding)
						it->second.setKey(targetUser);
					else
						it->second.setKey("");
					ModeHelperChannel(client, it, ch, adding, operatorPrivilege);
				}
				else
					SendToClient(client, ":" + this->_name + noOperatorPrivilege + client.getNick() + " " + target + ":you don’t have operator privileges to change modes\r\n");
			}
			else if (ch == 'o') // give operator status
			{
				if (adding)
				{
					if (it->second.isOperator(&client) && it->second.isMember(&client) && !targetUser.empty())
					{
						Client* newOperator = it->second.retrieveClient(targetUser);
						if (newOperator != nullptr)
							it->second.addOperator(&client, newOperator);
						else
							SendToClient(client, ":" + this->_name + " 401 " + client.getNick() + " " + target + ":no such nick or channel\r\n");
					}
					else
						SendToClient(client, ":" + this->_name + noOperatorPrivilege + client.getNick() + " " + target + ":you don’t have operator privileges to change modes\r\n");
				}
				else
				{
					if (it->second.isOperator(&client) && it->second.isMember(&client) && !targetUser.empty())
					{
						Client* possibleOperator = it->second.retrieveClient(targetUser);
						if (possibleOperator != nullptr)
							it->second.removeOperator(&client, possibleOperator, false);
						else
							SendToClient(client, ":" + this->_name + " 401 " + client.getNick() + " " + target + ":no such nick or channel\r\n");
					}
				}
			}
			else if (ch == 't') // change or view the channel topic
			{
				if (it->second.isOperator(&client) && it->second.isMember(&client))
					ModeHelperChannel(client, it, ch, adding, operatorPrivilege);
				else
					SendToClient(client, ":" + this->_name + noOperatorPrivilege + client.getNick() + " " + target + ":you don’t have operator privileges to change modes\r\n");
			}
			else if (ch == 'l') // set/remove user limit to channel
			{
				if (it->second.isOperator(&client) && it->second.isMember(&client))
				{
					ModeHelperChannel(client, it, ch, adding, operatorPrivilege);
					if (adding)
					{
						if (targetUser.empty())
							SendToClient(client, ":" + this->_name + " 461 " + client.getNick() + " " + ":not enough parameters\r\n");
						if (it->second.getMaxMembers() == true)
						{
							try {
								size_t pos;
								uint64_t nbMembers = std::stoull(targetUser, &pos);
								if (pos != targetUser.length())
								{
									SendToClient(client, ":" + this->_name + " 461 " + client.getNick() + " " + ":not enough parameters\r\n");
									return ;
								}
								if (nbMembers < it->second.getNbMembers())
									SendToClient(client, ":" + this->_name + " 471 " + client.getNick() + " " + ":channel is already over the limit\r\n");
								it->second.limitMaxMembers(nbMembers);
							} catch (const std::invalid_argument&)
							{
								SendToClient(client, ":" + this->_name + " 461 " + client.getNick() + " " + ":not enough parameters\r\n");
								return ;
							} catch (const std::out_of_range&) {
								SendToClient(client, ":" + this->_name + " 461 " + client.getNick() + " " + ":not enough parameters\r\n");
								return ;
							}
						}
					}
				}
				else
					SendToClient(client, ":" + this->_name + noOperatorPrivilege + client.getNick() + " " + target + ":you don’t have operator privileges to change modes\r\n");
			}
			else
			{ 
				// ERR_UMODEUNKNOWNFLAG
				SendToClient(client, ":" + this->_name + " 501 " + ":Unknown mode flag\r\n");
			}
		}
	}
	else
	{
		SendToClient(client, ":" + this->_name + " 221 " + "RPL_UMODEIS " + client.getModes() + "\r\n");
		return ;
	}
		
}

void Server::ModeHelperChannel(Client& client, std::map<std::string, Channel>::iterator it, char mode, bool adding, std::string code)
{
	if (adding)
	{
		it->second.setModes(mode); // +o, +k, +l need to be handled on server side. Class cannot handle.
		SendToClient(client, ":" + this->_name + code + it->first + " +" + mode + "\r\n");
	}
	else
	{
		it->second.removeMode(mode);
		SendToClient(client, ":" + this->_name + code + it->first + " -" + mode + "\r\n");
	}
}

void Server::Priv(Client& client, const std::string& message)
{
	std::stringstream stream(message);
	std::string command, target, messageContent;
	stream >> command >> target;
	if (target.empty())
	{
		// ERR_NEEDMOREPARAMS
		SendToClient(client, ":" + _name + " 461 " + " PRIVMSG :Not enough parameters\r\n");
		return;
	}

	std::getline(stream, messageContent);
	if (!messageContent.empty() && messageContent[0] == ' ')
		messageContent = messageContent.substr(1);
	if (!messageContent.empty() && messageContent[0] == ':')
        messageContent = messageContent.substr(1);

	if (target[0] == '#')
	{
		auto it = _channels.find(target);
		if (it == _channels.end())
		{
			// ERR_NOSUCHCHANNEL
			SendToClient(client, ":" + _name + " 403 " + client.getNick() + " " + target + ": Invalid channel name\r\n");
			return ;
		}
		if (it->second.isMember(&client))
			SendToChannel(target, ":" + client.getNick() + " PRIVMSG " + target + " :" + messageContent + "\r\n", &client, false);
		else
		{
			std::string ERR_NOMEMBER = "Zorg: you are not a member of the channel " + target + "\r\n";
			SendToClient(client, ERR_NOMEMBER);
			SendToClient(client, ":" + _name + " 442 " + client.getNick() + " " + target + ": you're not in the channel\r\n");
		}
	}
	else
	{
		auto it = std::find_if(_clients.begin(), _clients.end(), [&target](Client& c) {return c.getNick() == target; });
		
		if (it != _clients.end())
		{
			std::string formattedMessage = ":" + client.getNick() + " PRIVMSG " + target + " :" + messageContent + "\r\n";
			SendToClient(*it, formattedMessage);
		}
		else
			SendToClient(client, ":" + _name + " 401 " + client.getNick() + " " + target + " :No such nick/channel\r\n");
	}
}

void Server::Quit(Client& client, const std::string& message)
{
	std::string quitMessage = "[Zorg] Client has disconnected";
	if (!message.empty())
	{
		quitMessage = message;
		if (quitMessage[0] == ':')
			quitMessage = quitMessage.substr(1);
	}
	std::string quitBroadcast = ":" + client.getNick() + " QUIT :" + quitMessage + "\r\n";
	for (auto& c : _clients)
	{
		if (c.getNick() != client.getNick())
			SendToClient(c, quitBroadcast);
	}
	std::cout << "[Zorg] Client " << client.getNick() << " has disconnected\r\n";
}

void Server::Join(Client& client, const std::string& message)
{
	std::istringstream stream(message);
	std::string command, channel, key;
	stream >> command >> channel >> key;
	
	if (channel.empty() || channel[0] != '#')
	{
		// ERR_BADCHANMASK
		SendToClient(client, ":" + _name + " 476 " + client.getNick() + " " + channel + ":invalid channel name" + "\r\n");
		return ;
	}

	auto it = _channels.find(channel);
	if (it == _channels.end()) 
	{
		Channel newChannel(channel, key, "", false, false);
		_channels.emplace(channel, newChannel);
		it = _channels.find(channel);
	}

	// check if priovided key matches
    if (!it->second.getKey().empty() && it->second.getKey() != key)
    {
        SendToClient(client, ":" + _name + " 475 " + client.getNick() + " " + channel + " :bad channel key\r\n");
        return ;
    }

	// check if server has imposed limit
	if (it->second.getMaxMembers())
	{
		// check if there is space
		if ((it->second.getNbMembers() + 1) > it->second.getNumberMaxMembers())
		{
			SendToClient(client, ":" + _name + " 471 " + client.getNick() + " " + channel + " :channel is full\r\n");
			return ;
		}
	}

	it->second.addMember(&client);	
	if (it->second.isMember(&client))
	{
		SendToClient(client, ":" + client.getNick() + " JOIN " + channel + "\r\n");
		std::string namesList;
		for (Client* member : it->second.getMembers())
			namesList += member->getNick() + " ";
		SendToClient(client, ":" + _name + " 353 " + client.getNick() + " = " + channel + " :" + namesList + "\r\n");
		SendToClient(client, ":" + _name + " 366 " + client.getNick() + " " + channel + " :End of /NAMES list.\r\n");
		if (!it->second.getTopic().empty())
		{
			SendToClient(client, ":" + _name + " 332 " + client.getNick() + " " + channel + " :" + it->second.getTopic() + "\r\n");
			
			SendToClient(client, ":" + _name + " 333 " + client.getNick() + " " + channel + " " + it->second.getSetter() + " " + std::to_string(it->second.getTopicTime()) + "\r\n");
		}
	}
}

int Server::Pass(Client& client, const std::string& message){
	std::istringstream stream(message);
	std::string command, password;

	stream >> command >> password;
	if(client.getAuthentication()){
		SendToClient(client, ":" + _name + " 462 " + client.getNick() + " :You may not reregister\r\n");
		return 0;
	}

	if(password.empty()){
		SendToClient(client, ":" + _name + " 461 " + client.getNick() +  "PASS :Not enough parameters\r\n");
		return 0;
	}

	if(password != _password){
		SendToClient(client, ":" + _name + " 464 " + client.getNick() +  " :Password Incorrect\r\n");
		return 0;
	}
	// no response message in case of correct PASS
	std::cout << "[Zorg] password accepted" << std::endl;
	return 1;
}


void Server::SendToChannel(const std::string& channelName, const std::string& message, Client* sender, bool justJoined)
{
	auto it = _channels.find(channelName);
	if (it == _channels.end())
		return ;
	
	if (justJoined)
	{
		if (it->second.isMember(sender))
			SendToClient(*sender, message);
		return ;
	}

	for (Client* member : it->second.getMembers())
	{
		if (member != sender)
		{
			if (it->second.isMember(sender))
				SendToClient(*member, message);
		}
	}
}

void Server::Part(Client& client, const std::string& message)
{
	std::istringstream stream(message);
	std::string command, channel;
	stream >> command >> channel;

	if (channel.empty() || channel[0] != '#')
	{
		//ERR_BADCHANMASK
		SendToClient(client, ":" + _name + " 476 " + client.getNick() + " " + channel + ": invalid channel syntax name\r\n");
		return ;
	}

	auto it = _channels.find(channel);
	// we did not find any channel
	if (it == _channels.end()) 
	{
		//ERR_NOSUCHCHANNEL
		SendToClient(client, ":" + _name + " 403 " + client.getNick() + " " + channel + ": invalid channel name\r\n");
		return ;
	}
	if (it->second.isOperator(&client))
	{
		it->second.removeOperator(&client, nullptr, true);
		// case for when there are no operators left on the channel but there are still members in there
		if (it->second.noOperators())
		{
			for (Client* member : it->second.getMembers())
			{
				if (member->getClientFd() != client.getClientFd())
				{
					it->second.addOperator(nullptr, member);
					break ;
				}
			}
			SendToChannel(channel, ":" + client.getNick() + " PRIVMSG " + channel + " :" + client.getNick() + " was the last operator and left. First of the list has been made operator" + "\r\n", &client, true);
		}
	}
	if (it->second.isMember(&client))
	{
		it->second.removeMember(&client);
		SendToClient(client, ":" + client.getNick() + " PART " + channel + "\r\n");
	}
	else
		SendToChannel(channel, ":" + client.getNick() + " PRIVMSG " + channel + " :" + "cannot part because you are not a member" + "\r\n", &client, true);
	if (it->second.isChannelEmpty())
		_channels.erase(channel);
}

void	Server::Topic(Client& client, const std::string& message)
{
	std::vector<std::string> tokens;
	std::istringstream stream(message);
	std::string token;
	while (stream >> token)
		tokens.push_back(token);
	if (tokens.size() < 2)
	{
		SendToClient(client, ":" + _name + " 461 " + client.getNick() + " TOPIC :Not enough parameters\r\n");
		return ;
	}
	std::string channel_name = tokens[1];
	auto channelIt = _channels.find(channel_name);
	if (channelIt == _channels.end())
	{
		SendToClient(client, ":" + _name + " 403 " + client.getNick() + " " + channel_name + " :No such channel\r\n");
		return ;
	}
	Channel& channel = channelIt->second;
	if (!channel.isMember(&client))
	{
		SendToClient(client, ":" + _name + " 442 " + client.getNick() + " " + channel_name + " :You're not on that channel\r\n");
		return ;
	}
	if (tokens.size() == 2)
	{
		if (channel.getTopic().empty())
			SendToClient(client, ":" + _name + " 331 " + client.getNick() + " " + channel_name + " :No topic is set\r\n");
		else
			SendToClient(client, ":" + _name + " 332 " + client.getNick() + " " + channel_name + " :" + channel.getTopic() + "\r\n");
		return ;
	}
	if (!channel.isOperator(&client))
	{
		SendToClient(client, ":" + _name + " 482 " + client.getNick() + " " + channel_name + " :You're not a channel operator\r\n");
		return ;
	}
	std::string new_topic = message.substr(message.find(':') + 1);
	channel.setTopic(new_topic, client.getNick());
	for (Client* member : channel.getMembers())
	{
		SendToClient(*member, ":" + client.getNick() + "!" + client.getUser() + "@" + client.getHost() + " TOPIC " + channel_name + " :" + new_topic + "\r\n");
	}
}

void Server::Invite(Client& client, const std::string& message)
{
	std::istringstream stream(message);
	std::string command, nickname, channelName;
	stream >> command >> nickname >> channelName;

	if (nickname.empty() || channelName.empty())
	{
		SendToClient(client, ":" + _name + " 461 " + client.getNick() + " INVITE :Not enough parameters\r\n");
		return;
	}
	if (channelName[0] != '#')
	{
		SendToClient(client, ":" + _name + " 403 " + client.getNick() + " " + channelName + " :No such channel\r\n");
		return;
	}
	auto TargetIt = std::find_if(_clients.begin(), _clients.end(), [&nickname](Client& c) {return c.getNick() == nickname; });
	if (TargetIt == _clients.end())
	{
		SendToClient(client, ":" + _name + " 401 " + client.getNick() + " " + nickname + " :No such nick/channel\r\n");
		return;
	}
	auto ChannelIt = _channels.find(channelName);
	if (ChannelIt != _channels.end())
	{
		Channel& channel = ChannelIt->second;
		if (!channel.isMember(&client))
		{
			SendToClient(client, ":" + _name + " 442 " + client.getNick() + " " + channelName + " :You're not on that channel\r\n");
			return;
		}
		if (channel.isMember(&(*TargetIt)))
		{
			SendToClient(client, ":" + _name + " 443 " + client.getNick() + " " + nickname + " " + channelName + " :is already on channel\r\n");
			return;
		}
		if (channel.getIsInviteOnly() && !channel.isOperator(&client))
		{
			SendToClient(client, ":" + _name + " 482 " + client.getNick() + " " + channelName + " :You're not a channel operator\r\n");
		}
	}
	SendToClient(*TargetIt, ":" + client.getNick() + " INVITE " + nickname + " " + channelName + "\r\n");
	SendToClient(client, ":" + _name + " 341 " + client.getNick() + " " + nickname + " " + channelName + "\r\n");
}

void Server::Kick(Client& client, const std::string& message)
{
	std::istringstream stream(message);
	std::string command, channelNames, usersToKick, comment;
	stream >> command >> channelNames >> usersToKick >> comment;
	std::getline(stream, comment);
	if (!comment.empty() && comment[0] == ':')
		comment = comment.substr(1);
	else if (comment.empty())
		comment = client.getNick();
	if (channelNames.empty() || usersToKick.empty())
	{
		SendToClient(client, ":" + _name + " 476 " + client.getNick() + " " + channelNames + " :Invalid channel syntax\r\n");
		return;
	}
	std::vector<std::string> channels, users;
	std::istringstream channelStream(channelNames), userStream(usersToKick);
	std::string token;
	while (std::getline(channelStream, token, ','))
	{
		channels.push_back(token);
	}
	while (std::getline(userStream, token, ','))
	{
		users.push_back(token);
	}
	if (channels.size() != 1 && channels.size() != users.size())
	{
		SendToClient(client, ":" + _name + " 461 " + client.getNick() + " KICK :Mismatched channels and users\r\n");
		return;
	}
	for (size_t i = 0; i < users.size(); ++i)
	{
		std::cout << "Inside for loop" << std::endl;
		std::string channelName = (channels.size() == 1) ? channels[0] : channels[i];
		std::string userToKick = users[i];
		if (channelName[0] != '#')
		{
			SendToClient(client, ":" + _name + " 476 " + client.getNick() + " " + channelName + " :Invalid channel syntax\r\n");
			continue;
		}
		auto channelIt = _channels.find(channelName);
		if (channelIt == _channels.end())
		{
			SendToClient(client, ":" + _name + " 403 " + client.getNick() + " " + channelName + " :No such channel\r\n");
			continue;
		}
		Channel &channel = channelIt->second;
		if (!channel.isMember(&client))
		{
	        SendToClient(client, ":" + _name + " 442 " + client.getNick() + " " + channelName + " :You're not on that channel\r\n");
			continue;
		}
		if (!channel.isOperator(&client))
		{
			SendToClient(client, ":" + _name + " 482 " + client.getNick() + " " + channelName + " :You're not channel operator\r\n");
			continue;
		}
		auto targetIt = std::find_if(_clients.begin(), _clients.end(), [&userToKick](Client& c) {return c.getNick() == userToKick; });
		if (targetIt == _clients.end())
		{
			SendToClient(client, ":" + _name + " 401 " + client.getNick() + " " + userToKick + " :No such nick\r\n");
			continue;
		}
		Client& target = *targetIt;
		if (!channel.isMember(&target))
		{
			SendToClient(client, ":" + _name + " 441 " + client.getNick() + " " + userToKick + " " + channelName + " :They are not on that channel\r\n");
			continue;
		}
		std::string KickMessage = ":" + client.getNick() + "!" + client.getUser() + "@" + client.getHost() + " KICK " + channelName + " " + userToKick + " :" + comment + "\r\n";
		SendToChannel(channelName, KickMessage, &client, true);
		SendToClient(target, KickMessage);
		channel.removeMember(&target);
		if (channel.isOperator(&target)) {
			channel.removeOperator(&target, nullptr, true);
			if (channel.noOperators())
			{
				for (Client* member : channel.getMembers())
				{
					if (member->getClientFd() != client.getClientFd())
					{
						channel.addOperator(nullptr, member);
						SendToChannel(channelName, ":" + member->getNick() + " PRIVMSG " + channelName + " :The kicked user was an operator. " + member->getNick() + " has been promoted.\r\n", nullptr, true);
					}
				}
			}
		}
	}
}