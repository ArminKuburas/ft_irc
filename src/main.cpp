/* ****************************************************************************/
/*  ROFL:ROFL:ROFL:ROFL 													  */
/*          _^___      										 				  */
/* L     __/   [] \    										 			      */
/* LOL===__        \   			MY ROFLCOPTER GOES BRRRRRR				  	  */
/* L      \________]  					by fdessoy-				  			  */
/*         I   I     			(fdessoy-@student.hive.fi)				  	  */
/*        --------/   										  				  */
/* ****************************************************************************/

#include <iostream>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstdlib>
#include <cstring>
#include <unistd.h>

/**
 * TO DO 
 *  
 * 1. parse the port input to make sure we are receiving numerical 16 bits info;
 * 2. Check weird inputs to htons();
 * 3. Check with group about password policy; 
 * 4. encrypted port or no?
 * 5. port range? ---------> 6667
 */
bool	is_digit(std::string str)
{
	for (char ch : str)
	{
		if (!std::isdigit(ch))
			return (false);
	}
	return (true);
}

int main(int argc, char **argv)
{
	std::string usage = "usage: ./ircserv <port> <password>";
	if (argc == 3)
	{
		std::string port = argv[1];
		std::string password = argv[2];
		if (password.empty())
		{
			std::cout << "Error: password cannot be empty" << std::endl;
			std::cout << usage << std::endl;
			return (EXIT_FAILURE);
		} 
		if (port.empty())
		{
			std::cout << "Error: cannot be empty" << std::endl;
			std::cout << usage << std::endl;
			return (EXIT_FAILURE);
		}
		if (!is_digit(port))
		{
			std::cout << "Error: port must be only digits within range 1-65535" << std::endl;
			std::cout << usage << std::endl;
			return (EXIT_FAILURE);
		}
		int port_number = std::stoi(port);
		if (port_number < 1 && port_number > 65535)
		{
			std::cout << "here" << std::endl;
			std::cout << "Error: port number must be in the range 1-65535" << std::endl;
			return (EXIT_FAILURE);
		}
		int socket_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (socket_fd == -1)
		{
			perror("Socket creation failed");
			return (EXIT_FAILURE);
		}
		int flags = fcntl(socket_fd, F_GETFL);
		if (flags == -1)
		{
			perror("fcntl(F_GETFL) failed");
			close(socket_fd);
			return (EXIT_FAILURE);
		}
		if (fcntl(socket_fd, F_SETFL, flags | O_NONBLOCK) == -1)
		{
			perror("fcntl(F_SETFL) failed");
			close (socket_fd);
			return (EXIT_FAILURE);
		}
		
		std::cout << "Server is set to non-blocking mode" << std::endl;

		struct sockaddr_in server_addr;
		memset(&server_addr, 0, sizeof(server_addr));
		server_addr.sin_family = AF_INET;
		server_addr.sin_port = htons(port_number);
		server_addr.sin_addr.s_addr = INADDR_ANY;

		if (bind(socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
		{
			perror("bind failed");
			close(socket_fd);
			return (EXIT_FAILURE);
		}

		if (listen(socket_fd, 100) == -1)
		{
			perror("listening failed");
			close(socket_fd);
			return (EXIT_FAILURE);
		}

		std::cout << "Server is listening in non-blocking mode" << std::endl;
		
		int client_fd;
		while (true) // change condition to signal handler later
		{
			client_fd = accept(socket_fd, nullptr, nullptr);
			if (client_fd == -1)
			{
				if (errno == EAGAIN || errno == EWOULDBLOCK)
				{
					std::cout << "No incoming connections, retrying...." << std::endl;
					usleep(500000);
					continue ;
				}
				else
				{
					perror("accept failed");
					break ;
				}
			}
		}
		std::cout << "Client connected! File descriptor: " << client_fd << std::endl;

		// placeholder for client handling logic

		close(client_fd);
		std::cout << socket_fd << std::endl;
		close(socket_fd);
	}
	else
		std::cout << usage << std::endl;
	return (0);
}