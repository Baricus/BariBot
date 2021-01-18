/* IRCCorrelator.cpp - Miles Shamo
 *
 * This is the implementation file for
 * the IRCCorrelator, a glorified map
 * of IRC command strings to functions
 * handling those commands.  
 *
 * This file mainly implements the
 * constructor, where the map is
 * populated with each command.
 *
 * Each command is a lambda expression
 * with a matching argument signature.
 *
 * All return true if successful and false otherwise;
 * other necessary data is returned through the 
 * IRCResults struct in the function signature.
 *
 */

#include "IRCCorrelator.hpp"

#include "loginException.hpp"

// IRCBot included for use
#include "TIRCBot.hpp"

// TODO - delete
#include <iostream>

#include <regex>

// the constructor which populates the map
Twitch::IRCCorrelator::IRCCorrelator()
{
	// for reference in ALL commands:
	//	
	// The smatch's capture groups are as follows
	// [0] - Entire command (0-1)@TAGS (0-1):PREFIX COMMAND PARAMETERS :FINAL PARAMETER
	// [1] - All tags supplied (if any)
	// [2] - Prefix (if any)
	// [3] - Command (all caps or number if correct form; this is more general)
	// [4] - Parameters
	// [5] - Optional final parameter


	// NOTICE
	//
	// NOTICE commands are similar to PRIVMSG commands
	// in that they're a direct message to the client.  
	// However, they cannot be replied to.  
	//
	// All notices mark event's happening so they're important
	// to record, but often require no response.  Thus, unless
	// the message specifically requires a response, we only
	// record.
	SFM["NOTICE"] = [](std::smatch &Sm, Twitch::IRCBot *Caller) -> bool
	{
		if (Sm[5].str() == "Login authentication failed") // need to restart
		{
			// creates proper token path
			auto cPath = Poco::Path(Caller->Path);
			
			throw loginException("IRCCorrelator: AUTH FAILED", Caller, cPath);
		}

		return true;
	};


	// PING
	//
	// Ping commands are used to keep connection alive.  
	// It is simply a request for a matching "pong" response
	SFM["PING"] = [](std::smatch &Sm, Twitch::IRCBot *Caller) -> bool
	{
		// queues an output
		Caller->write("PONG :" + Sm[5].str());

		// temporary write
		Caller->write("PRIVMSG #baricus :Hi!  This is a Bot (not the streamer, just whats typing) that I'm working on.  I'm currently working on setting up actual commands and I thought I might as well stream it.  ");

		return true;
	};


	// PRIVMSG
	//
	// PRIVMSG is (since this is a client) a message sent to us,
	// either because it was sent into a channel, or sent directly.
	//
	SFM["PRIVMSG"] = [](std::smatch &sm, Twitch::IRCBot *Caller) -> bool
	{
		static const std::regex CommandMatch(R"(!(\w+)\s*(.*))", std::regex_constants::ECMAScript | std::regex_constants::optimize);

		// copy message body for static use matching
		std::string message(sm[5].str());

		std::smatch commandStringMatch;
			
		// if we have a command
		if(std::regex_match(message, commandStringMatch, CommandMatch))
		{
			auto iter = Caller->Commands.SFM.find(commandStringMatch[1].str());

			// if command doesn't exist
			if (iter == Caller->Commands.SFM.end())
			{
				// TODO - decide if I should do something
			}
			else
			{
				//if command returns false
				if(!iter->second(sm, commandStringMatch, Caller))
				{
					// TODO - log
				}
				else
				{
					// TODO - log
				}
			}
		}

		return true;

	};
}
