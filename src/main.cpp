/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: akuburas <akuburas@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/08 09:49:45 by akuburas          #+#    #+#             */
/*   Updated: 2025/01/08 09:49:46 by akuburas         ###   ########.fr       */
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
#include "../inc/Client.hpp"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <fcntl.h>
#include <poll.h>
# include "Message.hpp"


bool	isDigit(std::string str)
{
	for (char ch : str)
	{
		if (!std::isdigit(ch))
			return (false);
	}
	return (true);
}

bool	parsing(std::string port, std::string password)
{
	std::string usage = "usage: ./ircserv <port> <password>";
	if (password.empty())
	{
		std::cout << "Error: password cannot be empty" << std::endl;
		std::cout << usage << std::endl;
		return (false);
	} 
	if (port.empty())
	{
		std::cout << "Error: cannot be empty" << std::endl;
		std::cout << usage << std::endl;
		return (false);
	}
	if (!isDigit(port))
	{
		std::cout << "Error: port must be only digits within range 1-65535" << std::endl;
		std::cout << usage << std::endl;
		return (false);
	}
	return (true);
}

int main(int argc, char **argv)
{
	if (argc != 3)
	{
		std::cout << "usage: ./ircserv <port> <password>" << std::endl;
		return (EXIT_FAILURE);
	}
	std::string port = argv[1], password = argv[2];
	if (parsing(port, password) == false)
		return (EXIT_FAILURE);
	Server irc_server(std::stoi(port), password);
	int server_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (server_fd < 0)
	{
		perror("Socket creation failed");
		return (EXIT_FAILURE);
	}
	irc_server.setSocket(server_fd);
	int flags = fcntl(irc_server.getSocket(), F_GETFL);
	if (flags == -1)
	{
		perror("fcntl(F_GETFL) failed");
		close(irc_server.getSocket());
		return (EXIT_FAILURE);
	}
	if (fcntl(irc_server.getSocket(), F_SETFL, flags | O_NONBLOCK) == -1)
	{
		perror("fcntl(F_SETFL) failed");
		close (irc_server.getSocket());
		return (EXIT_FAILURE);
	}
	irc_server.setServerAddr();
	std::cout << "Server is set to non-blocking mode" << std::endl;
	if (bind(irc_server.getSocket(), (struct sockaddr *)&irc_server.getServerAddr(), sizeof(irc_server.getServerAddr())) < 0)
	{
		perror("bind failed");
		std::cout << "Error: " << errno << std::endl;
		close(irc_server.getSocket());
		return (EXIT_FAILURE);
	}
	if (listen(irc_server.getSocket(), 5) == -1)
	{
		perror("listening failed");
		std::cout << "Error: " << errno << std::endl;
		close(irc_server.getSocket());
		return (EXIT_FAILURE);
	}
	std::cout << "Server is listening in non-blocking mode" << std::endl;
	std::cout << "Port: " << port << std::endl;
	
	struct pollfd fds[1];
	fds[0].fd = server_fd;
	fds[0].events = POLLIN; // monitor for incoming connections

	while (true)
	{
		int poll_result = poll(fds, 1, -1); // wait indefinitely for an event
		if (poll_result < 0)
		{
			perror("poll failed");
			std::cout << "Error: " << errno << std::endl;
			close(server_fd);
			return (EXIT_FAILURE);
		}
		if (fds[0].revents & POLLIN)
		{
			struct sockaddr_in client_addr;
			socklen_t client_addr_len = sizeof(client_addr);
			int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_addr_len);
			if (client_fd < 0)
			{
				perror("accept failed");
				std::cout << "Error: " << errno << std::endl;
				close(server_fd);
				return (EXIT_FAILURE);
			}
			std::cout << "Client connected! File descriptor: " << client_fd << std::endl;
			char buffer[1024];
			ssize_t bytes_read = read(client_fd, buffer, sizeof(buffer) - 1);
			if (bytes_read < 0)
			{
				perror("read failure");
				std::cout << "Error: " << errno << std::endl;
				close(client_fd);
				close(server_fd);
				return (EXIT_FAILURE);
			}
			buffer[bytes_read] = '\0';
			//std::cout << "Message: " << buffer << std::endl;
			try {
				std::string rawMessage = std::string(buffer);
				rawMessage.pop_back();
				rawMessage += "\r\n";
				Message msg(rawMessage);
				std::cout << "Prefix:_" << msg.getPrefix() << "_\n";
				std::cout << "Command:_" << msg.getCommand() << "_\n";
				std::cout << "Params:_" << msg.getParams() << "_\n";
				std::cout << "Suffix:_" << msg.getSuffix() << "_\n";

				std::cout << "Serialized msg:\n";
				std::string serMSG = msg.serialize();
				std::cout << serMSG << "\n";
			} catch (const std::exception& e) {
				std::cerr << "Error: " << e.what() << "\n";
	}
			close(client_fd); // at this point we are taking one message and closing the fd
		}
	}

	irc_server.Run();	
	return (0);
}