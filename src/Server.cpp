/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pmarkaid <pmarkaid@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/08 09:49:38 by akuburas          #+#    #+#             */
/*   Updated: 2025/01/28 11:51:02 by pmarkaid         ###   ########.fr       */
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
	_commands["QUIT"] = [this](Client& client, const std::string& message) 		{Quit(client, message); };
	_commands["PASS"] = [this](Client& client, const std::string& message) 		{Pass(client, message); };
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
                    if (bytes_read <= 0) {
                        std::cout << "Client disconnected: " << fds[i].fd << std::endl;
                        close(fds[i].fd);
                        _clients.erase(std::remove_if(_clients.begin(), _clients.end(),
                            [fd = fds[i].fd](const Client& client) { return client.getClientFd() == fd; }),
                            _clients.end());
                        fds[i] = fds[--nfds];
                        --i;                
                    } else {
                        buffer[bytes_read] = '\0';
						std::string receivedData(buffer);

						// Identify which client sent the message using the fd
						auto client = std::find_if(_clients.begin(), _clients.end(), [fd = fds[i].fd](const Client& client) { return client.getClientFd() == fd; });
						if (client->getNick().empty()){
							int grant_access = connectionHandshake(*client, receivedData);
							if(!grant_access){
								shutdown(client->getClientFd(), SHUT_RDWR);
								close(client->getClientFd());
								_clients.erase(
								std::remove_if(
									_clients.begin(),
									_clients.end(),
									[fd = fds[i].fd](const Client& client) { return client.getClientFd() == fd; }),
								_clients.end());
								fds[i] = fds[--nfds];
								--i;
							}
						}
						else if (client != _clients.end()) {
							std::vector<std::string> messages = splitMessages(receivedData);
							for (const auto& message : messages)
							{
								std::cout  << fds[i].fd << " >> " << message << std::endl;
								handleMessage(*client, message);
								std::cout << std::endl;
							}
						}
					}
				}
			}
		}
	}
}

int Server::connectionHandshake(Client& client, const std::string& receivedData){
	std::cout << "connectionHandshake running" << std::endl;
	std::vector<std::string> messages = splitMessages(receivedData);

	for (const auto& message : messages)
	{
		std::istringstream stream(message);
		std::string command;
		stream >> command;
		// /connect 127.0.0.1 6667
		//std::cout  << client.getFds() << " >> " << message << std::endl;

		// Convert command to uppercase
		std::transform(command.begin(), command.end(), command.begin(), ::toupper);
		if(command == "CAP"){
			std::cout  << client.getClientFd() << " >> " << message << std::endl;
			Server::Cap(client, message);
			std::cout << std::endl;
		}
		else if(command == "PASS"){
			std::cout  << client.getClientFd() << " >> " << message << std::endl;
			int grant_access = Server::Pass(client, message);
			std::cout << std::endl;
			if(!grant_access){
				return 0;
			}
		}
		else if (command == "NICK"){
			std::cout  << client.getClientFd() << " >> " << message << std::endl;
			Server::Nick(client, message);
			std::cout << std::endl;
		}
		else if (command == "USER"){
			std::cout  << client.getClientFd() << " >> " << message << std::endl;
			Server::User(client, message);
			std::cout << std::endl;
		}
	}
	return 1;
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
	ssize_t bytes_sent = send(client.getClientFd(), message.c_str(), message.size(), 0);
	if (bytes_sent < 0) {
		perror("send failed");
	}
	else {
		std::cout << client.getClientFd() << " << " << message;
	}
}

void Server::Cap(Client& client, const std::string& message)
{
	(void)message;
	SendToClient(client, ":" +this->_name + " CAP * LS :*\r\n");
}

void Server::Nick(Client& client, const std::string& message)
{
	std::string nickname;
	std::string command;
	std::istringstream stream(message);
	stream >> command >> nickname;
	if (nickname.empty()) {
		SendToClient(client, ":" +this->_name + " 431 * NICK :No nickname given\r\n");
		return;
	}
	// Check if nickname is already in use
    for (const auto& existingClient : _clients) {
        if (existingClient.getNick() == nickname) {
            SendToClient(client, ":" + this->_name + " 433 * " + nickname + " :Nickname is already in use\r\n");
            return;
        }
    }

	// Check that the user has not been registered yet
	bool registered = client.getNick().empty();

	// set nick
	client.setNick(nickname);
	SendToClient(client, ":" + client.getNick() + "!" + client.getUser() + "@" + client.getHost() + " NICK " + client.getNick() + "\r\n");
	
	// welcome new users
	if(registered)
		SendToClient(client, ":" +this->_name + " 001 " + client.getNick() + " :Welcome to the IRC network, " + client.getNick() + "\r\n");
}

void Server::User(Client& client, const std::string& message)
{
	std::istringstream stream(message);
	std::string command, username, hostname, servername, realname;

	stream >> command >> username >> hostname >> servername;
	getline(stream, realname);
	if(!realname.empty() && realname[0] == ' ')
		realname = realname.substr(2);
	if (username.empty())
	{
		SendToClient(client, ":" +this->_name + " 461 * USER :Not enough parameters\r\n");
		return;
	}
	client.setUser(username);
	client.setRealname(realname);
	//std::cout << realname << std::endl;
}

void Server::Ping(Client& client, const std::string& message)
{
	size_t pos = message.find(" ");
	if (pos == std::string::npos || pos + 1 >= message.size())
	
	{
	    SendToClient(client, ":server 409 ERR_NOORIGIN :No origin specified\n");
		return;
	}
	std::string server1 = message.substr(pos + 1);
	if (server1.empty())
	{
		SendToClient(client, ":server 409 ERR_NOORIGIN :No origin specified\n");
		return;
	}
	//PONG response.
	SendToClient(client, "PONG " + server1 + "\n");
}

void Server::Mode(Client& client, const std::string& message)
{
	std::istringstream stream(message);
	std::string command, nickname, modeChanges;
	stream >> command >> nickname >> modeChanges;

	if (nickname != client.getNick())
	{
		SendToClient(client, ":server 502 ERR_USERSDONTMATCH :Cannot change mode for another user\n");
		return;
	}
	if (modeChanges.empty())
	{
		SendToClient(client, ":server 221 RPL_UMODEIS " + client.getModes() + "\n");
        return;
	}
	bool adding = true;
	for (char ch : modeChanges)
	{
		if (ch == '+')
			adding = true;
		else if (ch == '-')
			adding = false;
		else if (ch == 'i')
		{
			if (adding){
				client.addMode(ch);
				SendToClient(client, ":" + this->_name + " 221 " + client.getNick() + " +i\r\n");
			}
			else
				client.removeMode(ch);
		}
		else
			SendToClient(client, ":server 501 ERR_UMODEUNKNOWNFLAG :Unknown mode flag\n");
	}
}

void Server::Priv(Client& client, const std::string& message)
{
	std::stringstream stream(message);
	std::string command, target, messageContent;
	stream >> command >> target;
	if (target.empty())
	{
		SendToClient(client, "ERR_NEEDMOREPARAMS PRIVMSG :Not enough parameters\r\n");
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
			SendToClient(client, "ERR_NOSUCHCHANNEL " + target + " :No such channel\r\n");
			return ;
		}
		sendMessageToChannel(target, ":" + client.getNick() + " PRIVMSG " + target + " :" + messageContent + "\r\n", &client);
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
			SendToClient(client, "ERR_NOSUCHNICK " + target + " :No such nick/channel\r\n");
	}
}

void Server::Quit(Client& client, const std::string& message)
{
	std::string quitMessage = "Client has disconnected";
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
	std::cout << "Client " << client.getNick() << " has disconnected\n";
}

void Server::Join(Client& client, const std::string& message)
{
	std::istringstream stream(message);
	std::string command, channel, key;
	
	// need to parse when the user inputs a channel name without '#'
	stream >> command >> channel >> key;
	
	// simple error check
	if (channel.empty() || channel[0] != '#')
	{
		// ERR_BADCHANMASK
		SendToClient(client, ":Server 476 " + client.getNick() + " " + channel + ": Invalid channel name\r\n");
		return ;
	}

	auto it = _channels.find(channel);
	if (it == _channels.end()) // we did not find any channel
	{
		Channel newChannel(channel, "na", false, false);
		_channels.emplace(channel, newChannel);
		it = _channels.find(channel);
	}
	
	it->second.addMember(&client);
	SendToClient(client, ":" + client.getNick() + " JOIN " + channel + "\r\n");
	std::string namesList = "Server 353 " + client.getNick() + " = " + channel + " :"; // this is not working properly
	for (Client* member : it->second.getMembers())
	{
		namesList += member->getNick() + " ";
	}
	namesList += "\r\n";
	SendToClient(client, namesList);
	SendToClient(client, "Server 366 " + client.getNick() + " " + channel + " End of /NAMES list\r\n");
	if (it->second.getTopic() == "na") // no topic
        SendToClient(client, "Server 332 " + client.getNick() + " " + channel + " :" + it->second.getTopic() + "\r\n");
	else // sends topic
        SendToClient(client, "Server 331 " + client.getNick() + " " + channel + " :No topic is set\r\n");
}

int Server::Pass(Client& client, const std::string& message){
	std::istringstream stream(message);
	std::string command, password;

	stream >> command >> password;

	if(password.empty()){
		SendToClient(client, this->_name + " 461 " + client.getNick() +  "PASS :Not enough parameters\r\n");
		return 0;
	}
	if(password != this->_password){
		std::cout << "Password sent by irssi is: " << password << std::endl;
		SendToClient(client, this->_name + " 464 " + client.getNick() +  ":Password incorrect\r\n");
		return 0;
	}
	return 1;
	// no response message in case of correct PASS	
}

void Server::sendMessageToChannel(const std::string& channelName, const std::string& message, Client* sender)
{
	auto it = _channels.find(channelName);
	if (it == _channels.end()) // couldn't find channel name
		return ;
	
	for (Client* member : it->second.getMembers())
	{
		if (member != sender)
			SendToClient(*member, message);
	}
}
