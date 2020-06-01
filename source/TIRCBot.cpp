/* TIRCBot.cpp - Miles Shamo
 *
 * Implementation of the twitch-specific
 * IRC client
 */

#include "TIRCBot.hpp"

// extra includes for error reporting
#include <asio/buffer.hpp>
#include <asio/error_code.hpp>
#include <asio/read_until.hpp>
#include <iostream>
#include <fstream>

// needed to bind functions to make handlers
#include <functional> // std::bind

using std::cout;
using std::endl;
using std::bind;

// Constructor
// 
// Initilizes members with initialization lists
// and then connects the client to the given server
// and starts the listening loop
//
// To ensure each client cannot read and write to the server
// at the same time, each client get's its own strand.  
Twitch::IRCBot::IRCBot(asio::io_context &context, string serv, string portNum)
	:	Server(serv), PortNumber(portNum),
		_Strand(asio::make_strand(context)),
		IPresolver(context), TCPsocket(context),
		inString(), inBuffer(inString)
{
	// connects to server
	_connect(Server, PortNumber);

	// starts listener
	_start();
}


// _connect
//
// connect handles connecting to a given server.
// It is called on construction and if the connection
// is dropped (in exponential intervals).
void Twitch::IRCBot::_connect(string server, string portNum)
{
	// DNS lookup
	auto endpoints = IPresolver.resolve(server, portNum);

	// attempts to connect
	asio::connect(TCPsocket, endpoints);

	// writes "hello world" temporarily
	asio::write(TCPsocket, asio::buffer("Hello World!\n"), error);
}


// _start
//
// start binds and initalizes the handlers for each client.
// 
// For the purposes of a bot, we listen for an incoming IRC
// communication and then (possibly) respond to that message.
//
// Thus, we simply listen and then use the onMessage function
// to handle commands
void Twitch::IRCBot::_start()
{
	using namespace std::placeholders;

	// binds onMessage to the strand as a handler for the first read
	asio::async_read_until(TCPsocket, inBuffer, '\n', 
			[this](const asio::error_code &e, size_t size)
			{
				_onMessage(e, size);
			});
}


void Twitch::IRCBot::_onMessage(const asio::error_code &e, std::size_t size)
{
	// gets line from buffer and clears it
	string line = inString.substr(0, size-1);
	inString.erase(0, size);

	// temporary output
	cout << "Line: " << line << endl;
	cout << "remaining buffer: " << inString << endl;

	// resets handler	
	asio::async_read_until(TCPsocket, inBuffer, '\n', 
			[this](const asio::error_code &e, size_t size)
			{
				_onMessage(e, size);
			});
}
