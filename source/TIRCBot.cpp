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

#include <exception>
#include <asio/write.hpp>

// regex for parsing
#include <regex>

// chrono used for system time
#include <chrono>

// file input output and cout (cout eventually will be removed in favor of individual logs)
#include <iostream>
#include <fstream>


using std::endl;

using std::chrono::system_clock;
using std::ctime;

// Constructor
// 
// Initilizes members with initialization lists
// and then connects the client to the given server
// and starts the listening loop.  
//
// To ensure each client cannot read and write to the server
// at the same time, each client get's its own strand.  
Twitch::IRCBot::IRCBot(
		asio::io_context &context, 
		std::string serv, 
		std::string portNum,
		IRCCorrelator &IRCCor,
		CommandCorrelator &Comms,
		const Poco::Path dirPath)
	
	:	Context(context),
		Server(serv), PortNumber(portNum),
		_Strand(asio::make_strand(context)),
		IPresolver(context), TCPsocket(context),
		inString(new std::string()), 
		inBuffer(*inString),
		Path(dirPath),
		IRC(IRCCor),
		Commands(Comms)
{
	// opens log file
	auto logPath = dirPath;
	logPath.append("log.txt");


	log = std::ofstream(logPath.toString(), std::ios_base::app);

	auto time = system_clock::to_time_t(system_clock::now());

	log   << endl
		  << "Client "         << dirPath.getBaseName() << " startup" << endl
		  << "Log file opened" << endl
		  << ctime(&time)      << endl
		  << endl;

	// load token from file
	auto tokenPath = dirPath;
	tokenPath.append("token.tok");
	std::ifstream input(tokenPath.toString());

	input >> Token;
	
	input.close();

	log << "Loaded token for login as " << Token.username << endl;

	// connects bot after everything is set
	_connect();

	// starts client once connected
	start();
}


// Destructor
//
Twitch::IRCBot::~IRCBot()
{
	log.close();

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
		log << "Connection failed...retrying" << endl;

		// resets the socket
		TCPsocket.close();
		TCPsocket = asio::ip::tcp::socket(Context);

		// if we hit an arbitrary limit on time, just throw
		if (delay > 100)
		{
			log << "Failed to connect" << endl;
			
			throw std::runtime_error("Could not connect");
		}
		// if we failed to connect, retry on a falloff timer
		asio::steady_timer timer(Context, asio::chrono::seconds(delay));
		timer.wait();

		_connect(delay * 2 + 1);
	}
}


// start
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
	log << "Logging in as " << Token.username << endl;

	// queues up a first write for capability requesting, password (oauth), and nickname
	asio::async_write(TCPsocket, 
			asio::buffer("CAP REQ :twitch.tv/tags twitch.tv/commands twitch.tv/membership\r\n"
						 "PASS oauth:" + Token.accessToken + "\r\n"
						 "NICK " + Token.username + "\r\n"),
			asio::bind_executor(_Strand,

				// lambda to call after write completes
				[this](const asio::error_code &e, size_t size)
				{	
					// if nothing goes wrong
					if (!e)
					{
						log  << "login info sent as "    << Token.username << endl
							 << "Scopes requested are: " << endl
							 << "\t"                     << Token.scopes   << endl;

						// queue up an asyncronous read over the socket
						asio::async_read_until(
							this->TCPsocket, 
							this->inBuffer, 
							'\n', 
							asio::bind_executor(_Strand,
								// lambda to call the onMessage function
								[this](const asio::error_code &e, size_t size)
								{
									this->_onMessage(e, size);
								}
								));

						// opens channel lists
						auto chanPath = Path;
						chanPath.append("channels.txt");

						std::ifstream in(chanPath.toString());

						// joins each channel in the list
						std::string channel;
						int count = 0;
						while ((in >> channel))
						{
							write("JOIN #" + channel);
							count++;
						}
						
						log << "Joined " << count << " channels" << endl;
					}
					else
					{
						log  << "Login write failed with error:" << endl
							 << "\t"                             << e.value()   << endl
							 << "\t"                             << e.message() << endl;

						//TODO throw error upstream
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
		log   << "***message read failed with error:" << endl
			  << "\t"                                 << e.value()   << endl
			  << "\t"                                 << e.message() << endl;

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
	// end of the string to create a functionally equivalent regex.  
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
		log  << "Failed to parse IRC message: " << endl
			 << "\t"                            << line << endl;
	}
	else // we have a good line
	{
		log << "IRC " << sm[3] << " RECIEVED" << endl;

		// to handle commands, we use the IRC Correlator to find the proper function
		auto iter = IRC.SFM.find(sm[3].str());

		// if we can't find the command
		if (iter == IRC.SFM.end())
		{
			log << "\tNo command found." << endl;
		}
		else //else, we got our response
		{
			log << "\tCommand found:" << endl;
			// call the function related to the command
			log << "\t\t" << iter->second(sm, this) << endl;
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


// write
//
// write queues a message to asyncronously be written to
// the TCP socket (and properly logs it)
void Twitch::IRCBot::write(const std::string messageString)
{
	// the buffer needs a string which is gaurenteed to be in scope, so
	// we re-allocate on the heap as a string
	//
	// we also add proper line termination here to ensure it is only one place
	std::string *message = new std::string(messageString + "\r\n");
	
	// queues up write with a simple handler to write to logs and clean up
	asio::async_write(TCPsocket, asio::buffer(*message),
			asio::bind_executor(_Strand,
				[this, message](const asio::error_code &e, size_t size)
				{
					if (e)
					{
						log    << "Standard write failed with error:" << endl
							   << "\t"                                << e.value()   << endl
							   << "\t"                                << e.message() << endl;
					}

					// TODO - log message

					// as message is now used, we can delete it
					delete message;
				}
			));
}
