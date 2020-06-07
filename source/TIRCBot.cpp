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

using std::cout;
using std::endl;


// Constructor
// 
// Initilizes members with initialization lists
// and then connects the client to the given server
// and starts the listening loop.  
//
// To ensure each client cannot read and write to the server
// at the same time, each client get's its own strand.  
Twitch::IRCBot::IRCBot(asio::io_context &context, string serv, string portNum)
	:	Server(serv), PortNumber(portNum),
		_Strand(asio::make_strand(context)),
		IPresolver(context), TCPsocket(context),
		inString(), inBuffer(inString)
{
	// sets token to "NULL" to easily check if invalid
	Token = "NULL";

	// connects to server (if successful, updates flag)
	_connect(Server, PortNumber);
}


// _connect
//
// connect handles connecting to a given server.
// It is called on construction and if the connection
// needs to be re-opened.  
//
// connect simply opens the connection.  It does not
// authenticate, etc.  
void Twitch::IRCBot::_connect(string server, string portNum)
{
	// try catch for connection errors
	try
	{
		// DNS lookup
		auto endpoints = IPresolver.resolve(server, portNum, error);

		// attempts to connect
		asio::connect(TCPsocket, endpoints);

		// assuming we're here, the connection is active
	}
	catch (asio::system_error &e)
	{
		// TODO connect repeatedly on a falloff timer

		// for now, throw the error upstream
		throw e;
	}
}


// _authenticate
//
// authenticate sends two IRC messages to the Twitch
// server to properly establish the connection
void Twitch::IRCBot::_authenticate()
{
	// close connection if token is null
	if (Token == "NULL")
	{
		TCPsocket.close();
		return;
	}

	// writes two lines to the IRC, pass and nickname
	asio::write(TCPsocket, asio::buffer("PASS oauth:" + Token + "\n"));
	asio::write(TCPsocket, asio::buffer("NICK " + Username + "\n"));
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
void Twitch::IRCBot::start()
{
	// TODO log
	
	// binds onMessage to the strand as a handler for the first read
	asio::async_read_until(TCPsocket, inBuffer, '\n', 
			[this](const asio::error_code &e, size_t size)
			{
				_onMessage(e, size);
			});

	// TODO authenticate
	_authenticate();	
}

// _onMessage
//
// onMessage is a message handler that runs on every line recieved
// over the TCP socket.  It then can handle any and all interactions
// accordingly, usually by calling other functions
void Twitch::IRCBot::_onMessage(const asio::error_code &e, std::size_t size)
{
	if (e)
	{
		cout << "ERROR: " << e.message() << " | " << e.value() << endl;
		
		// ERR OUT, so shut down
		TCPsocket.close();

		return;
	}

	// gets line from buffer and clears it
	string line = inString.substr(0, size-1);
	inString.erase(0, size);

	// temporary output
	cout << "Line: " << line << endl;


	// resets handler	
	asio::async_read_until(TCPsocket, inBuffer, '\n', 
			[this](const asio::error_code &e, size_t size)
			{
				_onMessage(e, size);
			});
}


// giveToken
//
// giveToken is a standard "set" function to
// provide the instance with it's login token.
//
// This is used in authentificiation with Twitch's
// servers
void Twitch::IRCBot::giveToken(string tok)
{
	// TODO Error checking (look up twitch token requirements)

	Token = tok;
}

// giveUsername
//
// giveUsername is a "set" function to provide
// the instance with it's login username.
//
// This is used to authenticate with Twitch servers
void Twitch::IRCBot::giveUsername(string user)
{
	// TODO Error checking (not really necessary, but helpful)
	
	Username = user;
}
