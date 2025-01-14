/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: akuburas <akuburas@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/08 09:49:45 by akuburas          #+#    #+#             */
/*   Updated: 2025/01/09 05:20:21 by akuburas         ###   ########.fr       */
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
	int server_socket = irc_server.getSocket();
	int opt = 1;
	if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
	{
		perror("setsockopt failed");
		std::cout << "Error: " << errno << std::endl;
		close(server_socket);
		return (EXIT_FAILURE);
	}
	std::cout << "Server is set to non-blocking mode" << std::endl;
	if (bind(server_socket, (struct sockaddr *)&irc_server.getServerAddr(), sizeof(irc_server.getServerAddr())) < 0)
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

	irc_server.Run();	
	return (0);
}