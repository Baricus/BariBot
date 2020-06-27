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
			// TODO - add token index
			throw loginException("AUTH FAILED", Caller, Caller->Path);
		}

		return true;
	};


	// PING
	//
	// Ping commands are used to keep connection alive.  
	// It is simply a request for a matching "pong" response
	SFM["PING"] = [](std::smatch &Sm, Twitch::IRCBot *Caller) -> bool
	{
		
		//Res.output = "PONG :" + Sm[5].str();

		return true;
	};
}
