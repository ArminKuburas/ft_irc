/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fdessoy- <fdessoy-@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/08 09:49:38 by akuburas          #+#    #+#             */
/*   Updated: 2025/02/01 22:20:19 by fdessoy-         ###   ########.fr       */
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
	_commands["STATS"] = [this](Client& client, const std::string& message) 		{Stats(client, message); };
	_commands["WHOIS"] = [this](Client& client, const std::string& message) { Whois(client, message); };
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
	    SendToClient(client, ":server 409 ERR_NOORIGIN :No origin specified\r\n");
		return;
	}
	std::string server1 = message.substr(pos + 1);
	if (server1.empty())
	{
		SendToClient(client, ":server 409 ERR_NOORIGIN :No origin specified\r\n");
		return;
	}
	//PONG response.
	SendToClient(client, "PONG " + server1 + "\r\n");
}

void Server::Mode(Client& client, const std::string& message)
{
	std::istringstream stream(message);
	std::string command, target, modeChanges;
	stream >> command >> target >> modeChanges;
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
			const std::string serverName = it->second.getName();
			if (ch == '+')
				adding = true;
			else if (ch == '-')
				adding = false;
			else if (ch == 'i')
			{
				if (adding)
				{
					it->second.setModes(ch);
					std::cout << "adding mode to invite only" << std::endl;
					if (it->second.hasMode('i'))
						std::cout << "proof of added mode" << std::endl;
					SendToClient(client, ":" + this->_name + " (code for invite only) " + serverName + " +i\r\n");
				}
				else
					it->second.removeMode(ch);
			}
			else if (ch == 'k')
			{
				if (adding)
				{
					it->second.setModes(ch);
					std::cout << "adding mode to set a key" << std::endl;
					if (it->second.hasMode('k'))
						std::cout << "proof of added mode" << std::endl;
					SendToClient(client, ":" + this->_name + " (code for adding key) " + serverName + " +k\r\n");
				}
				else
					it->second.removeMode(ch);
			}
			else if (ch == 'o')
			{
				if (adding)
				{
					it->second.setModes(ch);
					std::cout << "adding mode for operator" << std::endl;
					if (it->second.hasMode('o'))
						std::cout << "proof of added mode" << std::endl;
					SendToClient(client, ":" + this->_name + " (code for adding operator status) " + serverName + " +o\r\n");
				}
				else
					it->second.removeMode(ch);
			}
			else if (ch == 't')
			{
				if (adding)
				{
					it->second.setModes(ch);
					std::cout << "adding mode to TOPIC restriction" << std::endl;
					if (it->second.hasMode('t'))
						std::cout << "proof of added mode" << std::endl;
					SendToClient(client, ":" + this->_name + " (code for adding restrictions on TOPIC) " + serverName + " +t\r\n");
				}
				else
					it->second.removeMode(ch);
			}
			else if (ch == 'l')
			{
				if (adding)
				{
					it->second.setModes(ch);
					std::cout << "adding mode for user limit in channel" << std::endl;
					if (it->second.hasMode('l'))
						std::cout << "proof of added mode" << std::endl;
					SendToClient(client, ":" + this->_name + " (code for adding user limit for channel) " + serverName + " +l\r\n");
				}
				else
					it->second.removeMode(ch);
			}
			else
			{
				// ERR_UMODEUNKNOWNFLAG
				SendToClient(client, ":" + _name + " 501 " + ":Unknown mode flag\r\n");
			}
		}
	}
	else
	{
		SendToClient(client, ":" + _name + " 221 " + "RPL_UMODEIS " + client.getModes() + "\r\n");
		return ;
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
			SendToClient(client, ":" + _name + " 401 " + target + " :No such nick/channel\r\n");
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
		SendToClient(client, ":" + _name + " 476 " + client.getNick() + " " + channel + ": invalid channel name" + "\r\n");
		return ;
	}

	auto it = _channels.find(channel);
	if (it == _channels.end()) 
	{
		Channel newChannel(channel, key, "na", false, false);
		_channels.emplace(channel, newChannel);
		it = _channels.find(channel);
	}
	if (it->second.getKey() == key)
		it->second.addMember(&client);
	else
	{
		SendToClient(client, ":" + _name + " 475 " + client.getNick() + " " + channel + ": invalid key" + "\r\n");
		return ;
	}
	if (it->second.isMember(&client))
	{
		SendToClient(client, ":" + client.getNick() + " JOIN " + channel + "\r\n");
		std::string namesList;
		for (Client* member : it->second.getMembers())
			namesList += member->getNick() + " ";
		SendToChannel(channel, ":" + client.getNick() + " PRIVMSG " + channel + " :" + "Members :" + namesList + "\r\n", &client, true);
		SendToChannel(channel, ":" + client.getNick() + " PRIVMSG " + channel + " :" + "Topic :" + it->second.getTopic() + "\r\n", &client, true);
		SendToChannel(channel, ":" + client.getNick() + " PRIVMSG " + channel + " :" + "has joined the channel\r\n", &client, false);
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
				// first possible members is the new operator
				// if (it->second.getMembers().empty())
				// {
				// 	_channels.erase(channel);
				// 	return ;
				// }
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
