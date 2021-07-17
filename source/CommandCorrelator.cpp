/* CommandCorrelator.cpp - Miles Shamo
 *
 * This file implements the Command Correlator,
 * an object designed to match strings to functions
 * to handle the associated command
 *
 *
 */

// for reference in ALL commands:
//	
// The IRCsm capture groups are as follows
// [0] - Entire command (0-1)@TAGS (0-1):PREFIX COMMAND PARAMETERS :FINAL PARAMETER
// [1] - All tags supplied (if any)
// [2] - Prefix (if any)
// [3] - Command (all caps or number if correct form; this is more general)
// [4] - Parameters
// [5] - Optional final parameter
//
// Note that for PRIVMSG IRC Commands (currently all commands)
// the 4'th smatch group (parameters) is just the channel.  
// Thus, we can use that to write to the channel that sent the
// message

#include "CommandCorrelator.hpp"
#include "TIRCBot.hpp"

#include <regex>
#include <sstream>
#include <iostream>

using namespace std;

// Constructor,
//
// handles population of the Command Correlator's objects
Twitch::CommandCorrelator::CommandCorrelator()
{
	misc();
	queue();
}

void Twitch::CommandCorrelator::misc()
{
	// test simply is a command to ensure the bot is online
	SFM["test"] = [](const std::smatch &IRCsm, 
			const std::smatch &Commandsm, 
			Twitch::IRCBot *Caller) -> std::string 
	{
		Caller->write("PRIVMSG " +  IRCsm[4].str() + " :This is a test");

		return R"(Command "test" fired)";
	};

	// echo repeats whatever the user provided in the arguments
	SFM["echo"] = [](const std::smatch &IRCsm, 
			const std::smatch &Commandsm, 
			Twitch::IRCBot *Caller) -> std::string 
	{
		Caller->write("PRIVMSG " +  IRCsm[4].str() + " :" + Commandsm[2].str());

		return R"(Command "echo" fired, echoed: )" + Commandsm[2].str();
	};

	// endorse says that a user is pretty cool
	SFM["endorse"] = [](const std::smatch &IRCsm, 
			const std::smatch &Commandsm, 
			Twitch::IRCBot *Caller) -> std::string 
	{
		Caller->write("PRIVMSG " +  IRCsm[4].str() + " :" + Commandsm[2].str() + " is pretty cool");

		return R"(Command "endorse" fired, endorsed: )" + Commandsm[2].str();
	};

	// purge deletes all messages by a user in the chat
	// it currently does not work
	// TODO
	SFM["purge"] = [](const std::smatch &IRCsm,
			const std::smatch &Commandsm,
			Twitch::IRCBot *Caller) -> std::string
	{
		// grab the prefix to grab the username
		auto prefix = IRCsm[2].str();

		const static std::regex getUsername(R"((\w+)!\1@\1.tmi.twitch.tv)");

		std::smatch prefixMatch;

		// tests prefix to grab username
		if (!std::regex_match(prefix, prefixMatch, getUsername))
		{
			Caller->write("PRIVMSG " + IRCsm[4].str() + " :" + "Cound not find user");

			return R"(Command "purge" could not find the user to purge)";
		}
		else
		{
			// timeout's user for one second
			Caller->write("@ban-duration=1 :tmi.twitch.tv CLEARCHAT #baricus :"
					+ prefixMatch[1].str());

			return R"(Command "purge" succesfully purged a user from the chat)";
		}
	};
}

// TODO - search tags for sender display name to register the queue with them
// 		- delete inactive queues?
// 		- make a sandwich
void Twitch::CommandCorrelator::queue()
{
	// starts a queue with a given name
	SFM["startQ"] = [this](const std::smatch &IRCsm,
			const std::smatch &Commandsm,
			Twitch::IRCBot *Caller) -> std::string
	{
		// ensures we have a name
		if (Commandsm[2].str() == "")
		{
			Caller->write("PRIVMSG" + IRCsm[4].str() + " :Specify a queue name");

			return R"(Command startQ did not create a queue: "" attempted as name)";
		}

		// creates it and assigns the user who made it to be the "creator"
		Queues[Commandsm[2].str()] = {.Creator = IRCsm[2].str()};
	

		Caller->write("PRIVMSG " + IRCsm[4].str()    + " :"
				+ "Created Queue: " + Commandsm[2].str());

		return R"(Command "startQ" created queue: )" + Commandsm[2].str();
	};

	// lists all active queues
	SFM["listQ"] = [this](const std::smatch &IRCsm,
			const std::smatch &Commandsm,
			Twitch::IRCBot *Caller) -> std::string
	{
		if (Queues.size() == 0)
		{
			Caller->write("PRIVMSG " + IRCsm[4].str() + " :There are no queues");
			return R"(Command listQ had no queues to list)";
		}

		stringstream list;
		list << "Queues: ";
		for (const auto &q : Queues)
		{
			list << q.first << " :: ";
		}
		Caller->write("PRIVMSG " + IRCsm[4].str() + " :" + list.str());

		return R"(Command "listQ" listed )" + std::to_string(Queues.size()) + R"( queues.)";
	};

	// join a queue
	SFM["joinQ"] = [this](const std::smatch &IRCsm,
			const std::smatch &Commandsm,
			Twitch::IRCBot *Caller) -> std::string
	{
		// find the queue and possibly add ourselves to the list
		auto iter = Queues.find(Commandsm[2].str());
		
		// not found
		if (iter == Queues.end())
		{
			Caller->write("PRIVMSG " + IRCsm[4].str() + " :That is not a queue!");
			return R"(Command "joinQ" called; argument was not an open queue)";
		}

		// parses creator to get the username
		string name(IRCsm[2].str());
		name = name.substr(0, name.find('!'));

		iter->second.Queue.push_back(name);
		return R"(Command "joinQ" needs a user, stupid")";
	};

	// TODO finish queue workings
	SFM["popQ"] = [this](const std::smatch &IRCsm,
			const std::smatch &Commandsm,
			Twitch::IRCBot *Caller) -> std::string
	{
		const static std::regex spliceNameFromNum(R"((.*?)(?: (\d+))?$)");
		string command(Commandsm[2].str());
		// separate name and num
		smatch sm;
		if (!regex_match(command, sm, spliceNameFromNum))
		{
			Caller->write("PRIVMSG " + IRCsm[4].str() + " :I can't pop that!");
		}
		

	};

	//SFM["peekQ"] = [this](const std::smatch &IRCsm,
			//const std::smatch &Commandsm,
			//Twitch::IRCBot *Caller) -> std::string
	//{
		//// find the queue and possibly add ourselves to the list
		//auto iter = Queues.find(Commandsm[2].str());
		
		//// not found
		//if (iter == Queues.end())
		//{
			//Caller->write("PRIVMSG " + IRCsm[4].str() + " :That is not a queue!");
			//return R"(Command "peekQ" called; argument was not an open queue)";
		//}

		//const static regex qCount

	//};
}
