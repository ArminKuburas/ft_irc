/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: akuburas <akuburas@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/08 09:49:38 by akuburas          #+#    #+#             */
/*   Updated: 2025/01/10 11:21:26 by akuburas         ###   ########.fr       */
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
	_commands["CAP"] = [this](Client& client, const std::string& message) {Cap(client, message); };
	_commands["NICK"] = [this](Client& client, const std::string& message) {Nick(client, message); };
	_commands["USER"] = [this](Client& client, const std::string& message) {User(client, message); };
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
						std::string message(buffer);
						std::cout << "Received message from client " << fds[i].fd << ": " << message;
						auto client = std::find_if(_clients.begin(), _clients.end(), [fd = fds[i].fd](const Client& client) { return client.getClientFd() == fd; });
						if (client != _clients.end()) {
							handleMessage(*client, message);
						}
					}
				}
			}
		}
	}
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
	SendToClient(client, ":server 002 " + nickname + " :Your host is server, running version 1.0");
    SendToClient(client, ":server 003 " + nickname + " :This server was created today");
    SendToClient(client, ":server 004 " + nickname + " server 1.0 iwso");
}

void Server::User(Client& client, const std::string& message)
{
	std::cout << "USER command received" << std::endl;
	SendToClient(client, ":server 375 " + client.getNick() + " :- Welcome to the IRC Server -");
    SendToClient(client, ":server 376 " + client.getNick() + " :End of MOTD");
	std::istringstream stream(message);
	std::string command, username, hostname, servername, realname;

	stream >> command >> username >> hostname >> servername >> realname;
	getline(stream, realname);
	if(!realname.empty() && realname[0] == ':')
		realname = realname.substr(1);
	if (username.empty())
	{
		SendToClient(client, ":server-name 461 * USER :Not enough parameters\r\n");
		return;
	}
	client.setUser(username);
	client.setRealname(realname);
	if (!client.getNick().empty())
	{
		SendToClient(client, ":server-name 001 " + client.getNick() + " :Welcome to the IRC network, " + client.getNick() + "\r\n");
	}
}