/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: akuburas <akuburas@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/08 09:49:38 by akuburas          #+#    #+#             */
/*   Updated: 2025/01/14 14:47:35 by akuburas         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

/* ****************************************************************************/
/*  ROFL:ROFL:ROFL:ROFL 													  */
/*          _^___      										 				  */
/* L     __/   [] \    										 			      */
/* LOL===__        \   			MY ROFLCOPTER GOES BRRRRRR				  	  */
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
	_commands["CAP"] = [this](Client& client, const std::string& message) 		 {Cap(client, message); };
	_commands["NICK"] = [this](Client& client, const std::string& message) 		{Nick(client, message); };
	_commands["USER"] = [this](Client& client, const std::string& message) 		{User(client, message); };
	_commands["PING"] = [this](Client& client, const std::string& message) 		{Ping(client, message); };
	_commands["MODE"] = [this](Client& client, const std::string& message) 		{Mode(client, message); };
	_commands["PRIVMSG"] = [this](Client& client, const std::string& message) 	{Priv(client, message); };
	_commands["QUIT"] = [this](Client& client, const std::string& message) 		{Quit(client, message); };
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

// void Server::setFdPoll()
// {
// 	this->_fds[0].fd = this->getSocket();
// 	this->_fds[0].events = POLLIN; 
// }

// pollfd *Server::getFdPoll()
// {	
// 	return (this->_fds);
// }

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
                    // New client connection
                    struct sockaddr_in client_addr;
                    socklen_t client_addr_len = sizeof(client_addr);
                    int client_fd = accept(this->_serverSocket, (struct sockaddr*)&client_addr, &client_addr_len);
                    if (client_fd < 0) {
                        perror("accept failed");
                        continue;
                    }

                    this->_clients.emplace_back(client_fd, client_addr);
                    fds[nfds].fd = client_fd;
                    fds[nfds].events = POLLIN;
                    const char * welcome_message = "Welcome to the server!\n";
                    send(client_fd, welcome_message, strlen(welcome_message), 0);
                    ++nfds;

                    std::cout << "New client connected: " << client_fd << std::endl;
                } else {
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
                        // Broadcast message to all clients
                        buffer[bytes_read] = '\0';
						std::string receivedData(buffer);
						std::cout << "Received message from client " << fds[i].fd << ": " << receivedData;
						auto client = std::find_if(_clients.begin(), _clients.end(), [fd = fds[i].fd](const Client& client) { return client.getClientFd() == fd; });
						if (client != _clients.end()) {
							std::vector<std::string> messages = splitMessages(receivedData);
							for (const auto& message : messages)
							{
								handleMessage(*client, message);
							}
						}
					}
				}
			}
		}
	}
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
		SendToClient(client, ":server-name 421 * " + command + " :Unknown command\r\n");
	}
}

void Server::SendToClient(Client& client, const std::string& message)
{
	ssize_t bytes_sent = send(client.getClientFd(), message.c_str(), message.size(), 0);
	if (bytes_sent < 0) {
		perror("send failed");
	}
	else {
		std::cout << "Sent message: " << message <<" to client " << client.getClientFd() << ": " << message;
	}
}

void Server::Cap(Client& client, const std::string& message)
{
	(void)message;
	std::cout << "CAP command received" << std::endl;
	SendToClient(client, ":server-name CAP * LS :*\r\n");
}

void Server::Nick(Client& client, const std::string& message)
{
	std::cout << "NICK command received" << std::endl;
	std::string nickname;
	std::string command;
	std::istringstream stream(message);
	stream >> command >> nickname;
	if (nickname.empty()) {
		SendToClient(client, ":server-name 431 * NICK :No nickname given\r\n");
		return;
	}
	client.setNick(nickname);
	SendToClient(client, ":server-name 001 " + nickname + " :Welcome to the IRC network, " + nickname + "\r\n");
}

void Server::User(Client& client, const std::string& message)
{
	std::cout << "USER command received" << std::endl;
	std::istringstream stream(message);
	std::string command, username, hostname, servername, realname;

	stream >> command >> username >> hostname >> servername;
	getline(stream, realname);
	if(!realname.empty() && realname[0] == ' ')
		realname = realname.substr(2);
	if (username.empty())
	{
		SendToClient(client, ":server-name 461 * USER :Not enough parameters\r\n");
		return;
	}
	client.setUser(username);
	client.setRealname(realname);
	std::cout << realname << std::endl;
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
			if (adding)
				client.addMode(ch);
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
	auto it = std::find_if(_clients.begin(), _clients.end(), [&target](Client& c) {return c.getNick() == target; });
	
	if (it != _clients.end())
	{
		std::string formattedMessage = ":" + client.getNick() + " PRIVMSG " + target + " :" + messageContent + "\r\n";
		SendToClient(*it, formattedMessage);
	}
	else
		SendToClient(client, "ERR_NOSUCHNICK " + target + " :No such nick/channel\r\n");
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
