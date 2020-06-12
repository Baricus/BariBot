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
	
	cout << "Initalizing BariBot" << endl;	

	// creates the overseer
	Twitch::Overseer BariBot;

	// set's client ID and secret
	BariBot.setAppCreds("", "");

	// creates the old token for this test
	Twitch::token tok;
	tok.accessToken = "";
	tok.refreshToken = "";
	
	tok.username = "Baricus";

	tok.scopes = "unused at pres";

	auto temp = BariBot.createClientInstance(tok, "google.com", "6667");

	cout << "starting context" << endl;

	// starts bot with one thread
	BariBot.setContext(1);

	return 0;
}
