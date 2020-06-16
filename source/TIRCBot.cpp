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

// file input output and cout (cout eventually will be removed in favor of individual logs)
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
Twitch::IRCBot::IRCBot(asio::io_context &context, string serv, string portNum,
		IRCCorrelator &IRCCor)
	:	Context(context),
		Server(serv), PortNumber(portNum),
		_Strand(asio::make_strand(context)),
		IPresolver(context), TCPsocket(context),
		inString(), inBuffer(inString),
		IRC(IRCCor)
{
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
	asio::write(TCPsocket, asio::buffer("PASS oauth:" + Token + "\r\n"));
	asio::write(TCPsocket, asio::buffer("NICK " + Username + "\r\n"));
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
	// binds onMessage to the strand as a handler for the first read
	asio::async_read_until(TCPsocket, inBuffer, '\n', 
			asio::bind_executor(_Strand,
				[this](const asio::error_code &e, size_t size)
				{
					this->_onMessage(e, size);
				}
				));

	_authenticate();	

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

	// gets line from buffer (keeping '\n') and clears it
	string line;
	line = inString.substr(0, size);
	inString.erase(0, size);

	// A regex expression to parse an IRC command into a smatch.  
	// The expression is in EMCAscript regex and is set to prefer
	// fast match times over memory/compile time (since it's static).
	//
	// This expression is based on a command by Garrett W. 
	// which can be found at https://regexr.com/39dn4
	//
	// This modified version (mainly to account for tags) is
	// located at https://regexr.com/56llm
	//
	// Due to errors in C++11 EMCAregex, the ^ and $ assertions
	// fail to properly function here.  Thus, we kept the \n on the
	// end of the string to create a functionally equivalent line.  
	// This could be fixed in C++17 with the multiline flag but,
	// as I already decided to stick with '11, we'll just deal with it.
	//
	// The smatch's capture groups are as follows
	// [0] - Entire command (0-1)@TAGS (0-1):PREFIX COMMAND PARAMETERS :FINAL PARAMETER
	// [1] - All tags supplied (if any)
	// [2] - Prefix (if any)
	// [3] - Command (all caps or number if correct form; this is more general)
	// [4] - Parameters
	// [5] - Optional final parameter
	const static std::regex IRCLine
		(R"Delim((?:@(\S+) +)?(?::(\S+) +)?(\S+)(?: +([^\n\r]+?)(?!:))?(?: :([^\n\r]+))?\r\n)Delim", 
		 std::regex_constants::ECMAScript | std::regex_constants::optimize);	
	
	// test the regex against the recieved line
	std::smatch sm;
	if (!std::regex_match(line, sm, IRCLine))
	{
		// we didn't match the line, so we discard it
		cout << "DISCARDED LINE: " << line;
		// TODO LOG FAILED PARSINGS
	}
	else // we have a good line
	{
		cout << "GOOD LINE: " << sm[0];
		cout << "Separated: " << sm[1] << " | " << sm[2] << " | " << sm[3] << " | " << sm[4] << " | " << sm[5] << endl;

		// to handle commands, we use the IRC Correlator to find the proper function
		auto iter = IRC.SFM.find(sm[3].str());

		// if we can't find the command
		if (iter == IRC.SFM.end())
		{
			// TODO - log failed IRC command
		}
		else //else, we got our response
		{
			// call the function related to the command
			IRCResults result;
			if (!iter->second(sm, result))
			{
				// TODO - log failure
			}
			else
			{
				// TODO - log success and result

				// analyze the result to know what to output, etc
				if (result.shouldOutput)
				{
					asio::write(TCPsocket, asio::buffer(result.output));
				}

				if (result.shouldClose)
				{
					// Do I want this?	
				}
			}
		}
	}

	// resets handler	
	asio::async_read_until(TCPsocket, inBuffer, '\n', 
			asio::bind_executor(_Strand,
				[this](const asio::error_code &e, size_t size)
				{
					this->_onMessage(e, size);
				}
				));
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
