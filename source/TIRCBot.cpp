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
Twitch::IRCBot::IRCBot(asio::io_context &context, std::string serv, std::string portNum,
		IRCCorrelator &IRCCor,
		CommandCorrelator &Comms,
		const Poco::Path filePath)
	:	Context(context),
		Server(serv), PortNumber(portNum),
		_Strand(asio::make_strand(context)),
		IPresolver(context), TCPsocket(context),
		inString(new std::string()), inBuffer(*inString),
		Path(filePath),
		IRC(IRCCor),
		Commands(Comms)
{
	_connect();
}


// Destructor
//
// this literally just frees one string, because
// for some reason inString had to be on the heap
// to make the inBuffer not explode
Twitch::IRCBot::~IRCBot()
{
	delete inString;
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

		// if we hit an arbitrary limit on time, just throw
		if (delay > 1000)
			throw std::runtime_error("Could not connect");

		// if we failed to connect, retry on a falloff timer
		asio::steady_timer timer(Context, asio::chrono::seconds(delay));
		timer.wait();

		_connect(delay * 2 + 1);
	}
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
	// queues up a first write for capability requesting, password (oauth), and nickname
	asio::async_write(TCPsocket, 
			asio::buffer("CAP REQ :twitch.tv/tags twitch.tv/commands twitch.tv/membership\r\n"
						 "PASS oauth:" + Token + "\r\n"
						 "NICK " + Username + "\r\n"),
			asio::bind_executor(_Strand,

				// lambda to call after write completes
				[this](const asio::error_code &e, size_t size)
				{
				
					// if nothing goes wrong
					if (!e)
					{
						// queue up an asyncronous read over the socket
						asio::async_read_until(
							this->TCPsocket, 
							this->inBuffer, 
							'\n', 
							asio::bind_executor(_Strand,
								[this](const asio::error_code &e, size_t size)
								{
									this->_onMessage(e, size);
								}
								));

						// write messages to join the necessary twitch chat(s)
						// temporary fix while determining best practice for storage of channels
						write("JOIN #baricus");	
					}
				}
			));
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
	std::string line;
	line = inString->substr(0, size);
	inString->erase(0, size);

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
	// end of the std::string to create a functionally equivalent line.  
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
		cout << "Separated: " << sm[1]  << " | " << sm[2] << " | " << sm[3] << " | " << sm[4] << " | " << sm[5] << endl;

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
			if (!iter->second(sm, this))
			{
				// TODO - log failure
			}
			else
			{
				// TODO - log success and result

			}
		}
	}

	// rebinds handler	
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
void Twitch::IRCBot::giveToken(std::string tok)
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
void Twitch::IRCBot::giveUsername(std::string user)
{
	// TODO Error checking (not really necessary, but helpful)
	
	Username = user;
}


// write
//
// write queues a message to asyncronously be written to
// the TCP socket (and properly logs it)
void Twitch::IRCBot::write(const std::string messageString)
{
	std::cout << "***WRITING MESSAGE: " << messageString << std::endl;

	// the buffer needs a string which is gaurenteed to be in scope, so
	// we re-allocate on the heap as a string
	//
	// we also add proper line termination here
	std::string *message = new std::string(messageString + "\r\n");
	
	// queues up write with a simple handler to write to logs and clean up
	asio::async_write(TCPsocket, asio::buffer(*message),
			asio::bind_executor(_Strand,
				[message](const asio::error_code &e, size_t size)
				{
					if (e)
					{
						// TODO - log error
					}

					// TODO - log message

					// as message is out of scope, we can delete it
					delete message;
				}
			));
}
