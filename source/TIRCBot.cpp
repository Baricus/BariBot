/* TIRCBot.cpp - Miles Shamo
 *
 * Implementation of the twitch-specific
 * IRC client
 */

#include "TIRCBot.hpp"

// extra includes are unecessary but auto generated
#include <asio/basic_waitable_timer.hpp>
#include <asio/bind_executor.hpp>
#include <asio/buffer.hpp>
#include <asio/error_code.hpp>
#include <asio/read_until.hpp>
#include <asio/steady_timer.hpp>

// regex for parsing
#include <exception>
#include <asio/write.hpp>
#include <regex>

#include <iostream>
#include <fstream>



//TODO - remove
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
	:	Context(context),
		Server(serv), PortNumber(portNum),
		_Strand(asio::make_strand(context)),
		IPresolver(context), TCPsocket(context),
		inString(), inBuffer(inString)
{
	// connects to server (if successful, updates flag)
	_connect();
}


// _connect
//
// connect handles connecting to a given server.
// It is called on construction and if the connection
// needs to be re-opened.  
//
// connect simply opens the connection.  It does not
// authenticate, etc. 
//
// seconds is used to delay repeated reconnects using
// asio async_wait 
void Twitch::IRCBot::_connect(long delay)
{
	try
	{

	// DNS lookup
	auto endpoints = this->IPresolver.resolve(Server, PortNumber, error);

	// attempts to connect
	asio::connect(this->TCPsocket, endpoints);
	}
	catch(...)
	{
		cout << "_connect catch retry" << endl;

		// resets the socket
		TCPsocket.close();
		TCPsocket = asio::ip::tcp::socket(Context);

		std::cout << "Retry wait time: " <<delay << std::endl;
		// if we hit an arbitrary limit on time, just throw
		if (delay > 1000)
			throw;

		// if we failed to connect, retry on a falloff timer
		asio::steady_timer timer(Context, asio::chrono::seconds(delay));
		timer.wait();

		_connect(delay * 2 + 1);
	}
}


// _authenticate
//
// authenticate sends two IRC messages to the Twitch
// server to properly establish the connection
void Twitch::IRCBot::_authenticate()
{
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
	try
	{
		// TODO log

		// connect to server
		_connect();

		// binds onMessage to the strand as a handler for the first read
		asio::async_read_until(TCPsocket, inBuffer, '\n', 
				[this](const asio::error_code &e, size_t size)
				{
				this->_onMessage(e, size);
				});

		// TODO authenticate
		_authenticate();	
	}
	// try catch to reconnect
	catch (std::system_error &e)
	{
		std::cout << "start catch: " << e.what() << endl;
	}
}

// _onMessage
//
// onMessage is a message handler that runs on every line recieved
// over the TCP socket.  It then can handle any and all interactions
// accordingly, usually by calling other functions
void Twitch::IRCBot::_onMessage(const asio::error_code &e, std::size_t size)
{
	// if we have an error, the socket is likely closed so we'll need a new one
	if (e)
	{
		// TODO - remove
		cout << "ERROR: " << e.message() << " | " << e.value() << endl;
		
		// ERR OUT, so shut down
		TCPsocket.close();

		// TODO - throw proper error upstream
		return;
	}

	// gets line from buffer and clears it
	string line = inString.substr(0, size-1);
	inString.erase(0, size);

	// temporary output
	cout << "Line: " << line << endl;

	// TODO parce IRC messages
	
	// a regex expression to parse an IRC command into a smatch.
	//
	// This expression is based on a command by Garrett W. 
	// which can be found at https://regexr.com/39dn4
	//
	// This modified version (mainly to account for tags) is
	// located at https://regexr.com/56llm
	//
	// The smatch's capture groups are as follows
	// [0] - Entire command
	// [1] - All tags supplied (if any)
	// [2] - Prefix (if any)
	// [3] - Command (all caps or number if correct form; this is more general)
	// [4] - Parameters
	// [5] - Optional final parameter
	const static std::regex IRCLine
		(R"(^(?:@(\S+) +)?(?::(\S+) +)?(\S+)(?: +(?!:)(.+?))?(?: +:(.+))?$)");	

	// resets handler	
	asio::async_read_until(TCPsocket, inBuffer, '\n', 
			[this](const asio::error_code &e, size_t size)
			{
				this->_onMessage(e, size);
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
