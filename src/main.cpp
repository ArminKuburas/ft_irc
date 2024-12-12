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
	if (argc == 3)
	{
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

		struct sockaddr_in server_addr, client_addr;
		socklen_t client_addr_len = sizeof(client_addr);
		memset(&server_addr, 0, sizeof(server_addr));
		server_addr.sin_family = AF_INET;
		server_addr.sin_port = htons(port_number);
		server_addr.sin_addr.s_addr = INADDR_ANY;
		char buffer[1024];

		if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
		{
			perror("bind failed");
			close(server_fd);
			return (EXIT_FAILURE);
		}

		if (listen(server_fd, 5) == -1)
		{
			perror("listening failed");
			close(server_fd);
			return (EXIT_FAILURE);
		}

		std::cout << "Server is listening in non-blocking mode" << std::endl;
		std::cout << "Port: " << port << std::endl;
		
		int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_addr_len);
		if (client_fd < 0)
		{
			perror("accept failed");
			close(server_fd);
			return (EXIT_FAILURE);
		}
		
		std::cout << "Client connected! File descriptor: " << client_fd << std::endl;
		

		ssize_t bytes_read = read(client_fd, buffer, sizeof(buffer) - 1);
		if (bytes_read < 0)
		{
			perror("read failure");
			close(client_fd);
			close(server_fd);
			return (EXIT_FAILURE);
		}
		buffer[bytes_read] = '\0';

		std::cout << "Message: " << buffer << std::endl;

		// placeholder for client handling logic

		close(client_fd);
		close(server_fd);
	}
	else
		std::cout << "usage: ./ircserv <port> <password>" << std::endl;
	return (0);
}