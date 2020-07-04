/* CommandCorrelator.cpp - Miles Shamo
 *
 * This file implements the Command Correlator,
 * an object designed to match strings to functions
 * to handle the associated command
 *
 *
 */

#include "CommandCorrelator.hpp"
#include "TIRCBot.hpp"

// Constructor,
//
// handles population of the Command Correlator's objects
Twitch::CommandCorrelator::CommandCorrelator()
{
	// for reference in ALL commands:
	//	
	// The IRCsm capture groups are as follows
	// [0] - Entire command (0-1)@TAGS (0-1):PREFIX COMMAND PARAMETERS :FINAL PARAMETER
	// [1] - All tags supplied (if any)
	// [2] - Prefix (if any)
	// [3] - Command (all caps or number if correct form; this is more general)
	// [4] - Parameters
	// [5] - Optional final parameter


	SFM["test"] = [](const std::smatch &IRCsm, 
					 const std::smatch &Commandsm, 
					 Twitch::IRCBot *Caller) -> bool 
	{
		Caller->write("PRIVMSG #baricus :This is a test");

		return true;
	};

	SFM["echo"] = [](const std::smatch &IRCsm, 
					 const std::smatch &Commandsm, 
					 Twitch::IRCBot *Caller) -> bool 
	{
		Caller->write("PRIVMSG #baricus :" + Commandsm[2].str());

		return true;
	};

	SFM["endorse"] = [](const std::smatch &IRCsm, 
						const std::smatch &Commandsm, 
						Twitch::IRCBot *Caller) -> bool 
	{
		Caller->write("PRIVMSG #baricus :" + Commandsm[2].str() + " is pretty cool");

		return true;
	};

	SFM["purge"] = [](const std::smatch &IRCsm,
					  const std::smatch &Commandsm,
					  Twitch::IRCBot *Caller) -> bool
	{
		// grab the prefix to grab the username
		auto prefix = IRCsm[2].str();

		const static std::regex getUsername(R"((\w+)!\1@\1.tmi.twitch.tv)");

		std::smatch prefixMatch;

		// tests prefix to grab username
		if (!std::regex_match(prefix, prefixMatch, getUsername))
		{
			return false;
		}
		else
		{
			// timeout's user for one second
			Caller->write("@ban-duration=1 :tmi.twitch.tv CLEARCHAT #baricus :" + prefixMatch[1].str());

			return true;
		}
	};


}
