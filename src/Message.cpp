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

// Getters
std::string Message::getPrefix()
{
	return (prefix_);
}

std::string Message::getCommand()
{
	return (command_);
}

std::string Message::getParams()
{
	return (params_);
}

std::string Message::getSuffix()
{
	return (suffix_);
}

std::string Message::getSender()
{
	return (sender_);
}

std::string Message::getRecipient()
{
	return (recipient_);
}
