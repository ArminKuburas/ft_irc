/* ****************************************************************************/
/*  ROFL:ROFL:ROFL:ROFL 													  */
/*          _^___      										 				  */
/* L     __/   [] \    										 			      */
/* LOL===__        \   			MY ROFLCOPTER GOES BRRRRRR				  	  */
/* L      \________]  					by fdessoy-				  			  */
/*         I   I     			(fdessoy-@student.hive.fi)				  	  */
/*        --------/   										  				  */
/* ****************************************************************************/

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <fcntl.h>
#include <poll.h>
# include "Message.hpp"

/**
 * TO DO 
 *  
 * 1. parse the port input to make sure we are receiving numerical 16 bits info;
 * 2. Check weird inputs to htons();
 * 3. Check with group about password policy; 
 * 4. encrypted port or no?
 * 5. port range? ---------> 6667
 */
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

int portConversion(std::string port)
{
	int port_number = std::stoi(port);
	if (port_number < 1 && port_number > 65535)
	{
		std::cout << "Error: port number must be in the range 1-65535" << std::endl;
		return (-1);
	}
	return (port_number);
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
	int port_number = portConversion(port);
	if (port_number == -1)
		return (EXIT_FAILURE);
	int server_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (server_fd < 0)
	{
		perror("Socket creation failed");
		return (EXIT_FAILURE);
	}
	int flags = fcntl(server_fd, F_GETFL);
	if (flags == -1)
	{
		perror("fcntl(F_GETFL) failed");
		close(server_fd);
		return (EXIT_FAILURE);
	}
	if (fcntl(server_fd, F_SETFL, flags | O_NONBLOCK) == -1)
	{
		perror("fcntl(F_SETFL) failed");
		close (server_fd);
		return (EXIT_FAILURE);
	}
	
	std::cout << "Server is set to non-blocking mode" << std::endl;
	struct sockaddr_in server_addr;
	
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port_number);
	server_addr.sin_addr.s_addr = INADDR_ANY;

	if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
	{
		perror("bind failed");
		std::cout << "Error: " << errno << std::endl;
		close(server_fd);
		return (EXIT_FAILURE);
	}
	if (listen(server_fd, 5) == -1)
	{
		perror("listening failed");
		std::cout << "Error: " << errno << std::endl;
		close(server_fd);
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
				std::string rawMessage = std::string(buffer) + "\r\n";
				Message msg(rawMessage);
				std::cout << "Prefix: " << msg.getPrefix() << "\n";
				std::cout << "Command: " << msg.getCommand() << "\n";
				std::cout << "Params: " << msg.getParams() << "\n";
				std::cout << "Suffix: " << msg.getSuffix() << "\n";
			} catch (const std::exception& e) {
				std::cerr << "Error: " << e.what() << "\n";
	}
			close(client_fd); // at this point we are taking one message and closing the fd
		}
	}

	close(server_fd);
	
	return (0);
}