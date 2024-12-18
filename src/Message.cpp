/* ****************************************************************************/
/*  ROFL:ROFL:ROFL:ROFL 													  */
/*          _^___      										 				  */
/* L     __/   [] \    										 			      */
/* LOL===__        \   			MY ROFLCOPTER GOES BRRRRRR				  	  */
/* L      \________]  					by fdessoy-				  			  */
/*         I   I     			(fdessoy-@student.hive.fi)				  	  */
/*        --------/   										  				  */
/* ****************************************************************************/

# include "Message.hpp"

// Constructor
Message::Message(std::string rawMessage)
{
	parseIncommingMessage(rawMessage);
}

void Message::parseIncommingMessage(std::string rawMessage)
{
	// Check for CRLF termination
	if (rawMessage.size() < 2 || rawMessage.substr(rawMessage.size() - 2) != "\r\n") {
		throw std::invalid_argument("Invalid message format (no CRLF at the end)");
	}
	// Remove CRLF
	rawMessage = rawMessage.substr(0, rawMessage.size() - 2);

	// Parse sections
	size_t pos;
	// Parse prefix
	if(rawMessage[0] == ':')
	{
		size_t spacePos = rawMessage.find(' ');
		if(spacePos == std::string::npos)
			throw std::invalid_argument("Invalid message format (no space after prefix)");
		prefix_ =  rawMessage.substr(1, spacePos - 1);
		pos = spacePos + 1;
	}

	// Parse command
	size_t spacePos = rawMessage.find(' ', pos);
	if(spacePos == std::string::npos)
	{
		command_ = rawMessage.substr(pos);
		return;
	}
	command_ = rawMessage.substr(pos, spacePos - pos);
	pos = spacePos + 1;

	// Parse params and body
	size_t colonPos = rawMessage.find(':', pos);
	if (colonPos != std::string::npos) {
		params_ = rawMessage.substr(pos, colonPos - pos - 1);
		suffix_ = rawMessage.substr(colonPos + 1);
	} else {
		params_ = rawMessage.substr(pos);
	}

}

std::string Message::serialize()
{
	// TODO: the message format differ based on the command
	std::string message = ":" + prefix_ + " " + command_ + " " + params_ + " :" + suffix_ + "\r\n";
	return message;
}

// Getters
std::string Message::getPrefix() const
{
	return (prefix_);
}

std::string Message::getCommand() const
{
	return (command_);
}

std::string Message::getParams() const
{
	return (params_);
}

std::string Message::getSuffix() const
{
	return (suffix_);
}

std::string Message::getSender() const
{
	return (sender_);
}

std::string Message::getRecipient() const
{
	return (recipient_);
}
