/* main.cpp - Miles Shamo
 *
 * This is the main program file for
 * BariBot, a Twitch-specific IRC bot.
 *
 */


#include <asio/io_context.hpp>
#include <iostream>


#include "InstanceOverseer.hpp"
#include "TIRCBot.hpp"
#include "token.hpp"


int main()
{
	using std::cout;
	using std::cin;
	using std::endl;

	// creates the overseer
	Twitch::Overseer BariBot;

	// runs it (why did I write this?)
	BariBot.run();

	return 0;
}
