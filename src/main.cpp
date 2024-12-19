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
	
	irc_server.setServerAddr();

	struct pollfd fds[1];
	fds[0].fd = irc_server.getSocket();
	fds[0].events = POLLIN; // monitor for incoming connections

	while (true)
	{
		int poll_result = poll(fds, 1, -1); // wait indefinitely for an event
		if (poll_result < 0)
		{
			perror("poll failed");
			std::cout << "Error: " << errno << std::endl;
			close(irc_server.getSocket());
			return (EXIT_FAILURE);
		}
		if (fds[0].revents & POLLIN)
		{
			struct sockaddr_in client_addr;
			socklen_t client_addr_len = sizeof(client_addr);
			int client_fd = accept(irc_server.getSocket(), (struct sockaddr*)&client_addr, &client_addr_len);
			if (client_fd < 0)
			{
				perror("accept failed");
				std::cout << "Error: " << errno << std::endl;
				close(irc_server.getSocket());
				return (EXIT_FAILURE);
			}
			client.setClientAddr(client_addr);
			client.setClientAddrLen(client_addr_len);
			std::cout << "Client connected! File descriptor: " << client_fd << std::endl;
			const char *message = "Hello, client! I am the server!";
			ssize_t bytes_sent = send(client_fd, message, strlen(message), 0);
			if (bytes_sent < 0)
			{
				perror("send failed");
				std::cout << "Error: " << errno << std::endl;
				close(client_fd);
				close(irc.getSocket());
				return (EXIT_FAILURE);
			}
			char buffer[1024];
			ssize_t bytes_read = read(client_fd, buffer, sizeof(buffer) - 1);
			if (bytes_read < 0)
			{
				perror("read failure");
				std::cout << "Error: " << errno << std::endl;
				close(client_fd);
				close(irc.getSocket());
				return (EXIT_FAILURE);
			}
			buffer[bytes_read] = '\0';
			std::cout << "Message: " << buffer << std::endl;
			close(client_fd); // at this point we are taking one message and closing the fd
		}
	}

	close(irc_server.getSocket());
	
	return (0);
}