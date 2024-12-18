/* ****************************************************************************/
/*  ROFL:ROFL:ROFL:ROFL 													  */
/*          _^___      										 				  */
/* L     __/   [] \    										 			      */
/* LOL===__        \   			MY ROFLCOPTER GOES BRRRRRR				  	  */
/* L      \________]  					by fdessoy-				  			  */
/*         I   I     			(fdessoy-@student.hive.fi)				  	  */
/*        --------/   										  				  */
/* ****************************************************************************/

# include <iostream>

class Message
{
	private:
		std::string prefix_;
		std::string command_;
		std::string params_;
		std::string suffix_;
		std::string sender_;
		std::string recipient_;

	public:
		Message(std::string rawMessage);
		std::string getPrefix();
		std::string getCommand();
		std::string getParams();
		std::string getSuffix();
		std::string getSender();
		std::string getRecipient();
		void parseIncommingMessage(std::string rawMessage);
		std::string serialize();
};