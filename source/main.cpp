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
using std::cin;
using std::endl;

int main()
{
	cout << "Initalizing BariBot" << endl;	
	
	asio::io_context io;

	Twitch::IRCBot client(io, "irc.chat.twitch.tv", "6667");

	// temporary way to load token and username until handler is created
	string token, usr;
	cin >> token;
	cin >> usr;

	client.giveToken(token);
	client.giveUsername(usr);
	client.start();

	io.run();

	return 0;
}
