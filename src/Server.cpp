/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pmarkaid <pmarkaid@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/08 09:49:38 by akuburas          #+#    #+#             */
/*   Updated: 2025/02/09 11:50:31 by pmarkaid         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

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
	_commands["PONG"] = [this](Client& client, const std::string& message) 		{Pong(client, message); };
	_commands["MODE"] = [this](Client& client, const std::string& message) 		{Mode(client, message); };
	_commands["PRIVMSG"] = [this](Client& client, const std::string& message) 	{Priv(client, message); };
	_commands["JOIN"] = [this](Client& client, const std::string& message)		{Join(client, message); };
	_commands["PART"] = [this](Client& client, const std::string& message)		{Part(client, message); };
	_commands["QUIT"] = [this](Client& client, const std::string& message) 		{Quit(client, message); };
	_commands["PASS"] = [this](Client& client, const std::string& message) 		{Pass(client, message); };
	_commands["STATS"] = [this](Client& client, const std::string& message) 	{Stats(client, message); };
	_commands["WHOIS"] = [this](Client& client, const std::string& message) 	{ Whois(client, message); };
	_commands["TOPIC"] = [this](Client& client, const std::string& message) 	{ Topic(client, message); };
	_commands["HELP"] = [this](Client& client, const std::string& message) 		{ Help(client, message); };
	_commands["INVITE"] = [this](Client& client, const std::string& message) 	{ Invite(client, message); };
	_commands["KICK"] = [this](Client& client, const std::string& message) 		{ Kick(client, message); };
	_commands["WHO"] = [this](Client& client, const std::string& message) 		{ Who(client, message); };
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

void Server::cleanupFd(struct pollfd* fds, int& nfds, int index) {
	// Reset the fd slot that's being removed
	fds[index].fd = -1;
	fds[index].events = 0;
	fds[index].revents = 0;

	// Move last fd to current position only if we're not at the last position
	if (index < nfds - 1) {
		fds[index] = fds[nfds - 1];
	}
	
	// Clear the last fd slot
	fds[nfds - 1].fd = -1;
	fds[nfds - 1].events = 0;
	fds[nfds - 1].revents = 0;
	--nfds;
}

// void debugPrintBytes(const char* buffer, ssize_t bytes_read) {
//     std::cout << "Received " << bytes_read << " bytes: ";
//     for (ssize_t i = 0; i < bytes_read; i++) {
//         unsigned char c = buffer[i];
//         if (c == '\r') std::cout << "\\r";
//         else if (c == '\n') std::cout << "\\n";
//         else std::cout << c;
//     }
//     std::cout << std::endl;
// }

void	Server::Run()
{
	struct pollfd fds[1024];
    int nfds = 1;

    fds[0].fd = this->_serverSocket;
    fds[0].events = POLLIN;

	// define the maximum interval of time between client connection check
	time_t lastCheck = std::time(NULL);
	const time_t checkInterval = 60;

    while (true) 
	{
		// poll returns either at any activity of checkInterval time has passed
        int poll_result = poll(fds, nfds, checkInterval * 1000);
        if (poll_result < 0) {
            perror("poll failed");
            return;
        }

		// check for dead clients
		time_t currentTime = std::time(NULL);
		if ((currentTime - lastCheck) >= checkInterval)
		{
			checkClientTimeouts();
			lastCheck = currentTime;
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
					 // Only disconnect on actual read errors
                    if (bytes_read < 0) {
						if (errno != EAGAIN && errno != EWOULDBLOCK) {
							std::cout << "[Zorg] Disconnecting client: " << fds[i].fd << std::endl;
							for (auto& client : _clients)
							{
								if(client.getClientFd() == fds[i].fd){
									disconnectClient(client, "Connection error");
									cleanupFd(fds, nfds, i);
									--i;
									break;
								}
							}
						}
						continue;
					} else if (bytes_read == 0) {
						// Client closed connection
						std::cout << "[Zorg] Client closed connection: " << fds[i].fd << std::endl;
						for (auto& client : _clients) {
							if(client.getClientFd() == fds[i].fd){
								disconnectClient(client, "Client closed connection");
								cleanupFd(fds, nfds, i);
								--i;
								break;
							}
						}
					} else {
						buffer[bytes_read] = '\0';
						std::string receivedData(buffer);
						//debugPrintBytes(buffer, bytes_read);
						// Identify which client sent the message using the fd

						auto client = std::find_if(_clients.begin(), _clients.end(), [fd = fds[i].fd](const Client& client) { return client.getClientFd() == fd; });
						// Append new data to client's buffer
						client->appendBuffer(buffer, bytes_read);
						client->setLastActivity();

						// Only process if we have a complete message
						if (client->hasCompleteMessage()) {
							std::string receivedData = client->getAndClearBuffer();
							std::vector<std::string> messages = splitMessages(receivedData);
							if (!client->getRegistration()) {
								int grant_access = connectionHandshake(*client, messages, fds[i].fd); // Pass individual message
								if (!grant_access) {
									disconnectClient(*client, "Invalid Password");
									cleanupFd(fds, nfds, i);
									--i;
									break;
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
}

int Server::connectionHandshake(Client& client, std::vector<std::string> messages, int fd) {

	// If no messages received, keep connection alive
	if (messages.empty()) {
		return 1;
	}

	std::cout << "[Zorg] Connection Handshake for Client: " << fd << std::endl;
	for(const std::string& message : messages) {
		std::istringstream stream(message);
		std::string command;
		stream >> command;
		std::transform(command.begin(), command.end(), command.begin(), ::toupper);
		
		std::cout << client.getClientFd() << " >> " << message << std::endl;

		if (command == "PASS") {
			int grant_access = Server::Pass(client, message);
			if (!grant_access) {
				return 0;
			}
			client.setAuthentication(true);
		} else if (command == "NICK") {
			// Only allow NICK/USER after PASS
			if (!client.getAuthentication())
				SendToClient(client, ":" + _name + " 451 * :You have not registered\r\n");
			else
				Server::Nick(client, message);
		} else if (command == "USER") {
			if (!client.getAuthentication())
				SendToClient(client, ":" + _name + " 451 * :You have not registered\r\n");
			else
				Server::User(client, message);
		} else if (command == "CAP") {
			Server::Cap(client, message);
		} else {
			SendToClient(client, ":" + _name + " 451 * :You have not registered\r\n");
		}
	}
	// Register user
	if(client.getAuthentication() && !client.getNick().empty() && !client.getUser().empty()){
		client.setRegistration(true);
		SendToClient(client, ":" + _name + " 001 " + client.getNick() + " :Welcome to the server\r\n");
		Help(client, "HELP");
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

void Server::disconnectClient(Client& client, const std::string& reason) {

	//broadcast QUIT to the rest of the users
	std::string quitBroadcast = ":" + client.getNick() + "!" + client.getUser() + "@" + client.getHost() + " QUIT :" + reason + "\r\n";
	for (auto& c : _clients) {
		if (c.getClientFd() != client.getClientFd()) {
			SendToClient(c, quitBroadcast);
		}
	}

	SendToClient(client, ":" + _name + " ERROR :Closing Link: " + client.getNick() + " " + reason + "\r\n");
	shutdown(client.getClientFd(), SHUT_WR);
	close(client.getClientFd());
	_clients.erase(std::remove_if(_clients.begin(), _clients.end(),
		[fd = client.getClientFd()](const Client& c) { return c.getClientFd() == fd; }), _clients.end());

}

void Server::checkClientTimeouts()
{
	time_t currentTime = time(NULL);
	const time_t timeout = 300; // 5 minutes in seconds
	const time_t pingTimeout = 10; // 10 seconds to respond to PING

	for (auto& client : _clients)
	{
		if (client.getAwaitingPong())
		{
			// If client hasn't responded to PING within 10 seconds
			if ((currentTime - client.getPingTime()) > pingTimeout)
			{
				disconnectClient(client, "Inactivity timeout");
				continue;
			}
		}
		// if more than 5 minutes since last activity sent PING
		if ((currentTime - client.getLastActivity()) > timeout){
			std::cout << "[Zorg] PING dead clients..." << std::endl;
			SendToClient(client, "PING " + client.getNick() + "\r\n");
			client.setPingStatus(true);
		}
	}
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

	// Check if nickname is already in use. Server name cannot be used
	for (const auto& existingClient : _clients) {
		if (existingClient.getNick() == newNickname || newNickname == _name){
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
	if (username.empty() || hostname.empty() || servername.empty() || realname.empty()) {
		SendToClient(client, ":" + _name + " 461 "+ client.getNick() + "USER :Not enough parameters\r\n");
		return;
	}

	// Remove leading space and colon from realname if present
	if(!realname.empty() && realname[0] == ' ')
		realname = realname.substr(1);
	if(!realname.empty() && realname[0] == ':')
		realname = realname.substr(1);

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
	std::istringstream stream(message);
	std::string command, target;
	stream >> command >> target;

	// No target for the PING
	if (target.empty())
	{
		SendToClient(client, ":" + _name + " 409 " + client.getNick() + " :No origin specified\r\n");
		return;
	}
	// If target is the server name, respond with PONG
	if (target == _name)
	{
		SendToClient(client, "PONG "+ target + "\r\n");
	}
	else
	{
		// Any other target (including client nicknames) should get ERR_NOSUCHSERVER
		SendToClient(client, ":" + _name + " 402 " + client.getNick() + " " + target + " :No such server\r\n");
	}
}

void Server::Pong(Client& client, const std::string& message)
{
	// no need to answer anything, just update ping status
	(void)message;
	client.setPingStatus(false);
}

void Server::Mode(Client& client, const std::string& message)
{
	std::istringstream stream(message);
	std::string command, target, modeChanges, targetUser;
	stream >> command >> target >> modeChanges >> targetUser;
	bool channel = false;
	bool user = false;

	// check if target is either user or channel
	if (modeChanges.empty() || target.empty())
		return ;
	if (target[0] == '#') // checking for channel
	{
		for (auto it = _channels.begin(); it != _channels.end(); ++it)
		{
			if (it->first == target)
			{
				channel = true;
				break ;
			}
		}
		if (channel == false)
		{
			SendToClient(client, ":" + _name + " 403 " + target +  ": Invalid channel name\r\n");
			return ;
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
			std::string operatorPrivilege = " 324 ";
			std::string noOperatorPrivilege = " 482 ";
			std::string messageSyntax = ":~" + client.getNick() + "!~" + client.getNick() + "@" + client.getHost() + " MODE " + target + " " + modeChanges;
			if (ch == '+')
				adding = true;
			else if (ch == '-')
				adding = false;
			else if (ch == 'i')
			{
				if (it->second.isOperator(&client) && it->second.isMember(&client) && targetUser.empty())
				{
					ModeHelperChannel(client, it, ch, adding, operatorPrivilege);
					SendToChannel(it->second.getName(), messageSyntax + "\r\n", &client, UNIVERSAL_MSG);
				}
				else
					SendToClient(client, ":" + this->_name + noOperatorPrivilege + client.getNick() + " " + target + " :you don’t have operator privileges to change modes\r\n");
			}
			else if (ch == 'k') // insert key to channel
			{
				if (it->second.isOperator(&client) && it->second.isMember(&client)) // pre-existing key needs a different treament for error
				{
					if (adding)
					{
						if (!it->second.getKey().empty() && targetUser.empty())
						{
							SendToClient(client, ":" + this->_name + " 467 " + client.getNick() + " " + target + ":key already set\r\n");
							return ;
						}
						it->second.setKey(targetUser);
						SendToChannel(it->second.getName(), messageSyntax + " " + targetUser + "\r\n", &client, UNIVERSAL_MSG);
					}
					else
					{
						if (!targetUser.empty())
							return ;
						it->second.setKey("");
						SendToChannel(it->second.getName(), messageSyntax + "\r\n", &client, UNIVERSAL_MSG);

					}
					ModeHelperChannel(client, it, ch, adding, operatorPrivilege);
				}
				else
					SendToClient(client, ":" + this->_name + noOperatorPrivilege + client.getNick() + " " + target + ":you don’t have operator privileges to change modes\r\n");
			}
			else if (ch == 'o') // give operator status
			{
				if (adding)
				{
					if (it->second.isOperator(&client) && it->second.isMember(&client) && !targetUser.empty())
					{
						Client* newOperator = it->second.retrieveClient(targetUser);
						if (it->second.isOperator(newOperator))
							return ;
						if (newOperator != nullptr)
						{
							it->second.addOperator(&client, newOperator);
							SendToChannel(it->second.getName(), messageSyntax + " " + targetUser + "\r\n", &client, UNIVERSAL_MSG);
						}
						else
							SendToClient(client, ":" + this->_name + " 401 " + client.getNick() + " " + target + ":no such nick or channel\r\n");
					}
					else
						SendToClient(client, ":" + this->_name + noOperatorPrivilege + client.getNick() + " " + target + ":you don’t have operator privileges to change modes\r\n");
				}
				else
				{
					if (it->second.isOperator(&client) && it->second.isMember(&client) && !targetUser.empty())
					{
						Client* possibleOperator = it->second.retrieveClient(targetUser);
						if (possibleOperator != nullptr)
						{
							it->second.removeOperator(&client, possibleOperator);
							SendToChannel(it->second.getName(), messageSyntax + " " + targetUser + "\r\n", &client, UNIVERSAL_MSG);
						}
						else
							SendToClient(client, ":" + this->_name + " 401 " + client.getNick() + " " + target + ":no such nick or channel\r\n");

					}
				}
			}
			else if (ch == 't') // change or view the channel topic
			{
				if (it->second.isOperator(&client) && it->second.isMember(&client) && targetUser.empty())
				{
					SendToChannel(it->second.getName(), messageSyntax + "\r\n", &client, UNIVERSAL_MSG);
					ModeHelperChannel(client, it, ch, adding, operatorPrivilege);
				}
				else
					SendToClient(client, ":" + this->_name + noOperatorPrivilege + client.getNick() + " " + target + ":you don’t have operator privileges to change modes\r\n");
			}
			else if (ch == 'l') // set/remove user limit to channel
			{
				if (it->second.isOperator(&client) && it->second.isMember(&client))
				{
					ModeHelperChannel(client, it, ch, adding, operatorPrivilege);
					if (adding)
					{
						if (targetUser.empty())
							SendToClient(client, ":" + this->_name + " 461 " + client.getNick() + " " + ":not enough parameters\r\n");
						if (it->second.getMaxMembers() == true)
						{
							try {
								size_t pos;
								uint64_t nbMembers = std::stoull(targetUser, &pos);
								if (pos != targetUser.length())
								{
									SendToClient(client, ":" + this->_name + " 461 " + client.getNick() + " " + ":not enough parameters\r\n");
									return ;
								}
								if (nbMembers < it->second.getNbMembers())
									SendToClient(client, ":" + this->_name + " 471 " + client.getNick() + " " + ":channel is already over the limit\r\n");
								it->second.limitMaxMembers(nbMembers);
								SendToChannel(it->second.getName(), messageSyntax + " " + std::to_string(nbMembers) + "\r\n", &client, UNIVERSAL_MSG);
							} catch (const std::invalid_argument&)
							{
								SendToClient(client, ":" + this->_name + " 461 " + client.getNick() + " " + ":not enough parameters\r\n");
								return ;
							} catch (const std::out_of_range&) {
								SendToClient(client, ":" + this->_name + " 461 " + client.getNick() + " " + ":not enough parameters\r\n");
								return ;
							}
						}
					}
				}
				else
					SendToClient(client, ":" + this->_name + noOperatorPrivilege + client.getNick() + " " + target + ":you don’t have operator privileges to change modes\r\n");
			}
			else
			{ 
				// ERR_UMODEUNKNOWNFLAG
				SendToClient(client, ":" + this->_name + " 501 " + ":Unknown mode flag\r\n");
			}
		}
	}
	else
	{
		SendToClient(client, ":" + this->_name + " 221 " + "RPL_UMODEIS " + client.getModes() + "\r\n");
		return ;
	}
		
}

void Server::ModeHelperChannel(Client& client, std::map<std::string, Channel>::iterator it, char mode, bool adding, std::string code)
{
	if (adding)
	{
		it->second.setModes(mode);
		SendToClient(client, ":" + this->_name + code + it->first + " +" + mode + "\r\n");
	}
	else
	{
		it->second.removeMode(mode);
		SendToClient(client, ":" + this->_name + code + it->first + " -" + mode + "\r\n");
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
		{
			SendToChannel(target, ":" + client.getNick() + " PRIVMSG " + target + " :" + messageContent + "\r\n", &client, NORMAL_MSG);
		}
		else
		{
			std::string ERR_NOMEMBER = "Zorg: you are not a member of the channel " + target + "\r\n";
			SendToClient(client, ERR_NOMEMBER);
			SendToClient(client, ":" + _name + " 442 " + client.getNick() + " " + target + ": you're not in the channel\r\n");
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
			SendToClient(client, ":" + _name + " 401 " + client.getNick() + " " + target + " :No such nick/channel\r\n");
	}
}

void Server::Quit(Client& client, const std::string& message)
{
	std::istringstream stream(message);
	std::string command, reason;
	stream >> command >> reason;

	std::string quitMessage = "Client has disconnected";
	if (!reason.empty())
	{
		quitMessage = reason;
		if (quitMessage[0] == ':')
			quitMessage = quitMessage.substr(1);
	}
	disconnectClient(client, quitMessage);
	std::cout << "[Zorg] Client " << client.getNick() << " has disconnected" << std::endl;;
}

void Server::Join(Client& client, const std::string& message)
{
	std::map<std::string, std::string>allChannels = MapChannels(message);
	if (allChannels.empty())
		return ;

	for (auto it_map = allChannels.begin();it_map != allChannels.end(); ++it_map)
	{
		std::string channel = it_map->first;
		std::string key = it_map->second;
		if (channel.empty() || channel[0] != '#')
		{
			// ERR_BADCHANMASK
			SendToClient(client, ":" + _name + " 476 " + client.getNick() + " " + channel + ":invalid channel name" + "\r\n");
			continue ;
		}

		// We look for existing channel, and if it does not exist, we create it
		auto [it, inserted] = _channels.try_emplace(channel, Channel(channel, key, "", false, false));
	
		// check if priovided key matches
    	if (!it->second.getKey().empty() && it->second.getKey() != key)
    	{
    	    SendToClient(client, ":" + _name + " 475 " + client.getNick() + " " + channel + " :bad channel key\r\n");
    	    continue ;
    	}
	
		// check if server has imposed limit
		if (it->second.getMaxMembers())
		{
			// check if there is space
			if (it->second.getNbMembers() >= it->second.getNumberMaxMembers())
			{
				SendToClient(client, ":" + _name + " 471 " + client.getNick() + " " + channel + " :channel is full\r\n");
				continue ;
			}
		}

		// avoid double join
		if (it->second.isMember(&client))
			continue ;

		it->second.addMember(&client);
		if (it->second.isMember(&client))
		{
			SendToClient(client, ":" + client.getNick() + " JOIN " + channel + "\r\n");
			std::string namesList;
			for (Client* member : it->second.getMembers())
			{
				if (it->second.isOperator(member))
					namesList += "@" + member->getNick() + " ";
				else
					namesList += member->getNick() + " ";
			}
			SendToClient(client, ":" + _name + " 353 " + client.getNick() + " = " + channel + " :" + namesList + "\r\n");
			SendToClient(client, ":" + _name + " 366 " + client.getNick() + " " + channel + " :End of /NAMES list.\r\n");
			if (!it->second.getTopic().empty())
			{
				SendToClient(client, ":" + _name + " 332 " + client.getNick() + " " + channel + " :" + it->second.getTopic() + "\r\n");
				
				SendToClient(client, ":" + _name + " 333 " + client.getNick() + " " + channel + " " + it->second.getSetter() + " " + std::to_string(it->second.getTopicTime()) + "\r\n");
			}
			else
			{
				SendToClient(client, ":" + _name + " 331 " + client.getNick() + " " + channel + " :No topic is set\r\n");
			}
			SendToChannel(it->second.getName(), ":" + client.getNick() + "!~" + client.getNick() + "@" + client.getHost() + " JOIN " + it->second.getName() + "\r\n", &client, NORMAL_MSG); // why does this crap is persisting?
			//:fdessoy!~fdessoy@87-92-251-103.rev.dnainternet.fi JOIN #SUPER_BBQ
		}
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
		SendToClient(client, ":" + _name + " 464 * :Password Incorrect\r\n");
		return 0;
	}
	// no response message in case of correct PASS
	std::cout << "[Zorg] password accepted" << std::endl;
	return 1;
}

/**
 * SendToChannel codes (last parameter):
 * 
 * 1. JUST_JOINED - simple just joined message;
 * 2. NORMAL_MSG - simple message that goes to the chat, but not sender
 * 3. UNIVERSAL_MSG - Includes members and sender.
 */
void Server::SendToChannel(const std::string& channelName, const std::string& message, Client* sender, int code)
{
	auto it = _channels.find(channelName);
	if (it == _channels.end())
		return ;
	

	switch (code) {
		case JUST_JOINED:
			if (it->second.isMember(sender))
				SendToClient(*sender, message);
			break ;
		case NORMAL_MSG:
			for (Client* member : it->second.getMembers())
			{
				if (member != sender)
				{
					if (it->second.isMember(sender))
						SendToClient(*member, message);
				}
			}
			break ;
		case UNIVERSAL_MSG:
			for (Client* member : it->second.getMembers())
			{
				if (it->second.isMember(sender))
					SendToClient(*member, message);
			}
			break ;
		default:
			break ;
	}
}

void Server::Part(Client& client, const std::string& message)
{
	std::map<std::string, std::string>allChannels = MapChannels(message);
	if (allChannels.empty())
		return ;

	for (auto it_map = allChannels.begin(); it_map != allChannels.end(); ++it_map)
	{
		std::string channel = it_map->first;
		std::string key = it_map->second;

		if (channel.empty() || channel[0] != '#')
		{
			// ERR_BADCHANMASK
			SendToClient(client, ":" + _name + " 476 " + client.getNick() + " " + channel + " :Invalid channel syntax name\r\n");
			continue ;
		}

		auto it = _channels.find(channel);
		if (it == _channels.end())
		{
			// ERR_NOSUCHCHANNEL
			SendToClient(client, ":" + _name + " 403 " + client.getNick() + " " + channel + " :No such channel\r\n");
			continue ;
		}

		if (!it->second.isMember(&client))
		{
			SendToClient(client, ":" + _name + " 442 " + client.getNick() + " " + channel + " :You're not on that channel\r\n");
			continue ;
		}
		std::string partSyntax = ":" + client.getNick() + "!~" + client.getNick() + "@" + client.getHost() + " PART " + channel + "\r\n";
		SendToChannel(channel, partSyntax, &client, NORMAL_MSG);
		if (it->second.isOperator(&client))
			it->second.removeOperator(&client, nullptr);
		it->second.removeMember(&client);
		SendToClient(client, partSyntax);
		if (it->second.isChannelEmpty())
			_channels.erase(channel);
	}
}

void	Server::Topic(Client& client, const std::string& message)
{
	std::vector<std::string> tokens;
	std::istringstream stream(message);
	std::string token;
	while (stream >> token)
		tokens.push_back(token);
	if (tokens.size() < 2)
	{
		SendToClient(client, ":" + _name + " 461 " + client.getNick() + " TOPIC :Not enough parameters\r\n");
		return ;
	}
	std::string channel_name = tokens[1];
	auto channelIt = _channels.find(channel_name);
	if (channelIt == _channels.end())
	{
		SendToClient(client, ":" + _name + " 403 " + client.getNick() + " " + channel_name + " :No such channel\r\n");
		return ;
	}
	Channel& channel = channelIt->second;
	if (!channel.isMember(&client))
	{
		SendToClient(client, ":" + _name + " 442 " + client.getNick() + " " + channel_name + " :You're not on that channel\r\n");
		return ;
	}
	if (tokens.size() == 2)
	{
		if (channel.getTopic().empty())
			SendToClient(client, ":" + _name + " 331 " + client.getNick() + " " + channel_name + " :No topic is set\r\n");
		else
			SendToClient(client, ":" + _name + " 332 " + client.getNick() + " " + channel_name + " :" + channel.getTopic() + "\r\n");
		return ;
	}
	if (!channel.isOperator(&client))
	{
		SendToClient(client, ":" + _name + " 482 " + client.getNick() + " " + channel_name + " :You're not a channel operator\r\n");
		return ;
	}
	std::string new_topic = message.substr(message.find(':') + 1);
	channel.setTopic(new_topic, client.getNick());
	for (Client* member : channel.getMembers())
	{
		SendToClient(*member, ":" + client.getNick() + "!" + client.getUser() + "@" + client.getHost() + " TOPIC " + channel_name + " :" + new_topic + "\r\n");
	}
}

void Server::Help(Client& client, const std::string& message) {
    std::istringstream stream(message);
    std::string command, topic;
    stream >> command >> topic;

    // Color code for yellow (8)
    std::string color = "\033[33m";
    std::string reset = "\033[0m";

    if (topic.empty()) {
        // General help message using NOTICE instead of numeric replies
        SendToClient(client, ":" + _name + " NOTICE " + client.getNick() + " :Available commands:\r\n");
        SendToClient(client, ":" + _name + " NOTICE " + client.getNick() + " :  NICK     - Change your nickname\r\n");
        SendToClient(client, ":" + _name + " NOTICE " + client.getNick() + " :  USER     - Set your username and real name\r\n");
        SendToClient(client, ":" + _name + " NOTICE " + client.getNick() + " :  JOIN     - Join a channel\r\n");
        SendToClient(client, ":" + _name + " NOTICE " + client.getNick() + " :  PART     - Leave a channel\r\n");
        SendToClient(client, ":" + _name + " NOTICE " + client.getNick() + " :  PRIVMSG  - Send a message to a user or channel\r\n");
        SendToClient(client, ":" + _name + " NOTICE " + client.getNick() + " :  MODE     - Set user or channel modes\r\n");
        SendToClient(client, ":" + _name + " NOTICE " + client.getNick() + " :  TOPIC    - View or change channel topic\r\n");
        SendToClient(client, ":" + _name + " NOTICE " + client.getNick() + " :  WHOIS    - Get information about a user\r\n");
        SendToClient(client, ":" + _name + " NOTICE " + client.getNick() + " :  QUIT     - Disconnect from the server\r\n");
        SendToClient(client, ":" + _name + " NOTICE " + client.getNick() + " :  HELP     - Show this help message" + reset + "\r\n");
        SendToClient(client, ":" + _name + " NOTICE " + client.getNick() + " :Type HELP <command> for more information about a specific command\r\n");
		SendToClient(client, ":" + _name + " NOTICE " + client.getNick() + " :Note: In irssi, commands must be prepended with /quote (e.g., /quote NICK)\r\n");
        return;
    }

    // Convert topic to uppercase for case-insensitive comparison
    std::transform(topic.begin(), topic.end(), topic.begin(), ::toupper);

    // Specific command help using NOTICE
    if (topic == "NICK") {
        SendToClient(client, ":" + _name + " NOTICE " + client.getNick() + " :" + color + "NICK <nickname>" + reset + "\r\n");
        SendToClient(client, ":" + _name + " NOTICE " + client.getNick() + " :Changes your nickname. Requirements:\r\n");
        SendToClient(client, ":" + _name + " NOTICE " + client.getNick() + " :  - Must be unique\r\n");
        SendToClient(client, ":" + _name + " NOTICE " + client.getNick() + " :  - Max length: 9 characters\r\n");
        SendToClient(client, ":" + _name + " NOTICE " + client.getNick() + " :  - Must start with a letter\r\n");
        SendToClient(client, ":" + _name + " NOTICE " + client.getNick() + " :  - Can contain: letters, numbers, and [-_[]\\`^{}]\r\n");
    }
    else if (topic == "USER") {
        SendToClient(client, ":" + _name + " NOTICE " + client.getNick() + " :" + color + "USER <username> <hostname> <servername> <realname>" + reset + "\r\n");
        SendToClient(client, ":" + _name + " NOTICE " + client.getNick() + " :Sets your username and real name during registration.\r\n");
        SendToClient(client, ":" + _name + " NOTICE " + client.getNick() + " :Can only be used once during initial connection.\r\n");
    }
    else if (topic == "JOIN") {
        SendToClient(client, ":" + _name + " NOTICE " + client.getNick() + " :" + color + "JOIN <channel> [key]" + reset + "\r\n");
        SendToClient(client, ":" + _name + " NOTICE " + client.getNick() + " :Joins a channel. If it doesn't exist, creates it.\r\n");
        SendToClient(client, ":" + _name + " NOTICE " + client.getNick() + " :Channel names must start with # and may require a key.\r\n");
        SendToClient(client, ":" + _name + " NOTICE " + client.getNick() + " :First user to join becomes channel operator.\r\n");
    }
    else if (topic == "PART") {
        SendToClient(client, ":" + _name + " NOTICE " + client.getNick() + " :" + color + "PART <channel>" + reset + "\r\n");
        SendToClient(client, ":" + _name + " NOTICE " + client.getNick() + " :Leaves the specified channel.\r\n");
        SendToClient(client, ":" + _name + " NOTICE " + client.getNick() + " :You must be a member of the channel to leave it.\r\n");
    }
    else if (topic == "PRIVMSG") {
        SendToClient(client, ":" + _name + " NOTICE " + client.getNick() + " :" + color + "PRIVMSG <target> :<message>" + reset + "\r\n");
        SendToClient(client, ":" + _name + " NOTICE " + client.getNick() + " :Sends a private message to a user or channel.\r\n");
        SendToClient(client, ":" + _name + " NOTICE " + client.getNick() + " :Target can be a nickname or channel name.\r\n");
        SendToClient(client, ":" + _name + " NOTICE " + client.getNick() + " :For channels, you must be a member to send messages.\r\n");
    }
    else if (topic == "MODE") {
        SendToClient(client, ":" + _name + " NOTICE " + client.getNick() + " :" + color + "MODE <target> <modes> [parameters]" + reset + "\r\n");
        SendToClient(client, ":" + _name + " NOTICE " + client.getNick() + " :Sets modes for users or channels.\r\n");
        SendToClient(client, ":" + _name + " NOTICE " + client.getNick() + " :User modes:\r\n");
        SendToClient(client, ":" + _name + " NOTICE " + client.getNick() + " :  +i : Marks user as invisible\r\n");
        SendToClient(client, ":" + _name + " NOTICE " + client.getNick() + " :Channel modes:\r\n");
        SendToClient(client, ":" + _name + " NOTICE " + client.getNick() + " :  +i : Invite-only channel\r\n");
        SendToClient(client, ":" + _name + " NOTICE " + client.getNick() + " :  +t : Protected topic (only ops can change)\r\n");
        SendToClient(client, ":" + _name + " NOTICE " + client.getNick() + " :  +k : Set/remove channel key (password)\r\n");
        SendToClient(client, ":" + _name + " NOTICE " + client.getNick() + " :  +o : Give/take channel operator status\r\n");
        SendToClient(client, ":" + _name + " NOTICE " + client.getNick() + " :  +l : Set/remove user limit\r\n");
    }
    else if (topic == "TOPIC") {
        SendToClient(client, ":" + _name + " NOTICE " + client.getNick() + " :" + color + "TOPIC <channel> [:<new topic>]" + reset + "\r\n");
        SendToClient(client, ":" + _name + " NOTICE " + client.getNick() + " :Views or changes channel topic.\r\n");
        SendToClient(client, ":" + _name + " NOTICE " + client.getNick() + " :Without new topic, shows current topic.\r\n");
        SendToClient(client, ":" + _name + " NOTICE " + client.getNick() + " :With mode +t, only channel operators can change topic.\r\n");
    }
    else if (topic == "WHOIS") {
        SendToClient(client, ":" + _name + " NOTICE " + client.getNick() + " :" + color + "WHOIS <nickname>" + reset + "\r\n");
        SendToClient(client, ":" + _name + " NOTICE " + client.getNick() + " :Shows information about a user:\r\n");
        SendToClient(client, ":" + _name + " NOTICE " + client.getNick() + " :  - Nickname and username\r\n");
        SendToClient(client, ":" + _name + " NOTICE " + client.getNick() + " :  - Real name\r\n");
        SendToClient(client, ":" + _name + " NOTICE " + client.getNick() + " :  - Server information\r\n");
    }
    else if (topic == "QUIT") {
        SendToClient(client, ":" + _name + " NOTICE " + client.getNick() + " :" + color + "QUIT [:<message>]" + reset + "\r\n");
        SendToClient(client, ":" + _name + " NOTICE " + client.getNick() + " :Disconnects from the server.\r\n");
        SendToClient(client, ":" + _name + " NOTICE " + client.getNick() + " :Optional quit message will be shown to other users.\r\n");
    }
	else if (topic == "STATS") {
        SendToClient(client, ":" + _name + " NOTICE " + client.getNick() + " :" + color + "STATS N" + reset + "\r\n");
        SendToClient(client, ":" + _name + " NOTICE " + client.getNick() + " :Shows list of current clients connected to the server.\r\n");
        SendToClient(client, ":" + _name + " NOTICE " + client.getNick() + " :Currently only supports the N flag.\r\n");
    }
    else if (topic == "HELP") {
        SendToClient(client, ":" + _name + " NOTICE " + client.getNick() + " :" + color + "HELP [command]" + reset + "\r\n");
        SendToClient(client, ":" + _name + " NOTICE " + client.getNick() + " :Shows help information.\r\n");
        SendToClient(client, ":" + _name + " NOTICE " + client.getNick() + " :Without parameters, lists all available commands.\r\n");
        SendToClient(client, ":" + _name + " NOTICE " + client.getNick() + " :With a command name, shows detailed help for that command.\r\n");
    }
    else {
        SendToClient(client, ":" + _name + " NOTICE " + client.getNick() + " :No help available for: " + topic + "\r\n");
    }
}


void Server::Invite(Client& client, const std::string& message)
{
	std::istringstream stream(message);
	std::string command, nickname, channelName;
	stream >> command >> nickname >> channelName;

	if (nickname.empty() || channelName.empty())
	{
		SendToClient(client, ":" + _name + " 461 " + client.getNick() + " INVITE :Not enough parameters\r\n");
		return;
	}
	if (channelName[0] != '#')
	{
		SendToClient(client, ":" + _name + " 403 " + client.getNick() + " " + channelName + " :No such channel\r\n");
		return;
	}
	auto TargetIt = std::find_if(_clients.begin(), _clients.end(), [&nickname](Client& c) {return c.getNick() == nickname; });
	if (TargetIt == _clients.end())
	{
		SendToClient(client, ":" + _name + " 401 " + client.getNick() + " " + nickname + " :No such nick/channel\r\n");
		return;
	}
	auto ChannelIt = _channels.find(channelName);
	if (ChannelIt != _channels.end())
	{
		Channel& channel = ChannelIt->second;
		if (!channel.isMember(&client))
		{
			SendToClient(client, ":" + _name + " 442 " + client.getNick() + " " + channelName + " :You're not on that channel\r\n");
			return;
		}
		if (channel.isMember(&(*TargetIt)))
		{
			SendToClient(client, ":" + _name + " 443 " + client.getNick() + " " + nickname + " " + channelName + " :is already on channel\r\n");
			return;
		}
		if (channel.getIsInviteOnly() && !channel.isOperator(&client))
		{
			SendToClient(client, ":" + _name + " 482 " + client.getNick() + " " + channelName + " :You're not a channel operator\r\n");
		}
	}
	SendToClient(*TargetIt, ":" + client.getNick() + " INVITE " + nickname + " " + channelName + "\r\n");
	SendToClient(client, ":" + _name + " 341 " + client.getNick() + " " + nickname + " " + channelName + "\r\n");
}

void Server::Kick(Client& client, const std::string& message)
{
	std::istringstream stream(message);
	std::string command, channelNames, usersToKick, comment;
	stream >> command >> channelNames >> usersToKick >> comment;
	std::getline(stream, comment);
	if (!comment.empty() && comment[0] == ':')
		comment = comment.substr(1);
	else if (comment.empty())
		comment = client.getNick();
	if (channelNames.empty() || usersToKick.empty())
	{
		SendToClient(client, ":" + _name + " 476 " + client.getNick() + " " + channelNames + " :Invalid channel syntax\r\n");
		return;
	}
	std::vector<std::string> channels, users;
	std::istringstream channelStream(channelNames), userStream(usersToKick);
	std::string token;
	while (std::getline(channelStream, token, ','))
	{
		channels.push_back(token);
	}
	while (std::getline(userStream, token, ','))
	{
		users.push_back(token);
	}
	if (channels.size() != 1 && channels.size() != users.size())
	{
		SendToClient(client, ":" + _name + " 461 " + client.getNick() + " KICK :Mismatched channels and users\r\n");
		return;
	}
	for (size_t i = 0; i < users.size(); ++i)
	{
		std::cout << "Inside for loop" << std::endl;
		std::string channelName = (channels.size() == 1) ? channels[0] : channels[i];
		std::string userToKick = users[i];
		if (channelName[0] != '#')
		{
			SendToClient(client, ":" + _name + " 476 " + client.getNick() + " " + channelName + " :Invalid channel syntax\r\n");
			continue;
		}
		auto channelIt = _channels.find(channelName);
		if (channelIt == _channels.end())
		{
			SendToClient(client, ":" + _name + " 403 " + client.getNick() + " " + channelName + " :No such channel\r\n");
			continue;
		}
		Channel &channel = channelIt->second;
		if (!channel.isMember(&client))
		{
	        SendToClient(client, ":" + _name + " 442 " + client.getNick() + " " + channelName + " :You're not on that channel\r\n");
			continue;
		}
		if (!channel.isOperator(&client))
		{
			SendToClient(client, ":" + _name + " 482 " + client.getNick() + " " + channelName + " :You're not channel operator\r\n");
			continue;
		}
		auto targetIt = std::find_if(_clients.begin(), _clients.end(), [&userToKick](Client& c) {return c.getNick() == userToKick; });
		if (targetIt == _clients.end())
		{
			SendToClient(client, ":" + _name + " 401 " + client.getNick() + " " + userToKick + " :No such nick\r\n");
			continue;
		}
		Client& target = *targetIt;
		if (!channel.isMember(&target))
		{
			SendToClient(client, ":" + _name + " 441 " + client.getNick() + " " + userToKick + " " + channelName + " :They are not on that channel\r\n");
			continue;
		}
		std::string KickMessage = ":" + client.getNick() + "!" + client.getUser() + "@" + client.getHost() + " KICK " + channelName + " " + userToKick + " :" + comment + "\r\n";
		SendToChannel(channelName, KickMessage, &client, true);
		SendToClient(target, KickMessage);
		channel.removeMember(&target);
		if (channel.isOperator(&target)) {
			channel.removeOperator(&target, nullptr);
			if (channel.noOperators())
			{
				for (Client* member : channel.getMembers())
				{
					if (member->getClientFd() != client.getClientFd())
					{
						channel.addOperator(nullptr, member);
						SendToChannel(channelName, ":" + member->getNick() + " PRIVMSG " + channelName + " :The kicked user was an operator. " + member->getNick() + " has been promoted.\r\n", nullptr, true);
					}
				}
			}
		}
	}
}

void Server::Who(Client& client, const std::string& message)
{
	std::istringstream stream(message);
	std::string command, mask;
	stream >> command >> mask;
	if (mask.empty())
	{
		SendToClient(client, ":" + _name + " 315 " + client.getNick() + " :End of /WHO list\r\n");
		return;
	}
	if (mask[0] == '#')
	{
		auto it = _channels.find(mask);
		if (it == _channels.end())
		{
			SendToClient(client, ":" + _name + " 315 " + client.getNick() + " :End of /WHO list\r\n");
			return;
		}
		Channel& channel = it->second;
		for (Client* member : channel.getMembers())
		{
			std::string status = channel.isOperator(member) ? "H@" : "H";
			std::string response = ":" + _name + " 352 " + client.getNick() + " " + mask + " " + member->getUser() + " " + member->getHost() + " " + _name + " " + member->getNick() + " " + status + " :0 " + member->getRealname() + "\r\n";
			SendToClient(client, response);
		}
		SendToClient(client, ":" + _name + " 315 " + client.getNick() + " :End of /WHO list\r\n");
	}
	else
	{
		SendToClient(client, ":" + _name + " 315 " + client.getNick() + " :End of /WHO list\r\n");
	}
}

std::map<std::string, std::string>	Server::MapChannels( const std::string& message )
{
	std::istringstream 					stream(message);
	std::map<std::string, std::string>	channelMap;
	std::string command,channelsList,keysList;

	// this will never happen, but might as well check
    if (!(stream >> command) || (command != "PART" && command != "JOIN"))
		return {};

    if (!(stream >> channelsList))
		return {};

    std::getline(stream, keysList);
	if (!keysList.empty() && keysList[0] == ' ')
		keysList.erase(0, 1);

	std::vector<std::string> channels, keys;
	std::stringstream chStream(channelsList), keyStream(keysList);

	std::string token;
	while (std::getline(chStream, token, ','))
		channels.push_back(token);

	while (std::getline(keyStream, token, ','))
		keys.push_back(token);

	for (size_t i = 0; i < channels.size(); ++i)
	{
		std::string respectiveKey = (i < keys.size()) ? keys[i] : "";
		channelMap[channels[i]] = respectiveKey;
	}
	return (channelMap);
}
