/* main.cpp - Miles Shamo
 *
 * This is the main program file for
 * BariBot, a Twitch-specific IRC bot.
 *
 */

#include <asio/io_context.hpp>
#include <iostream>

#include "TIRCBot.hpp"

using std::cout;
using std::endl;

int main()
{
	cout << "Initalizing BariBot" << endl;	
	
	asio::io_context io;

	Twitch::IRCBot client(io, "irc.chat.twitch.tv", "6667");
	client.giveToken("[REDACTED]");
	client.start();

	io.run();

	return 0;
}
