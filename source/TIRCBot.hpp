/* TIRCBot.hpp - Miles Shamo
 *
 * A definition of a C++ class that
 * manages an IRC connection intended
 * to be directed towards Twitch serevers
 */

#ifndef TWITCH_TIRCBOT
#define TWITCH_TIRCBOT

#include <asio/buffer.hpp>
#include <asio/io_context.hpp>

#include <Poco/Path.h>

#include <cstdlib>
#include <string>
#include <vector>
#include <map>

// for smatch
#include <regex>

#include <asio.hpp>

// analysis of IRC commands
#include "IRCCorrelator.hpp"


namespace Twitch
{
	class IRCBot
	{
		private:
			// Socket and other network objects
		
			// io context refference
			asio::io_context &Context;

			// connection data
			std::string Server, PortNumber;

			// strand for handle execution (currently unneeded)
			asio::strand<asio::io_context::executor_type> _Strand;

			// resolver
			asio::ip::tcp::resolver IPresolver;

			//basic stream socket Pointer
			asio::ip::tcp::socket TCPsocket;

			//ASIO error system
			asio::error_code error;

			//read buffer (string has to be heap for...reasons??)
			std::string *inString;
			asio::dynamic_string_buffer
			<std::string::value_type, std::string::traits_type, std::string::allocator_type>
				inBuffer;

			// All non-network related members

			// token file reference for thrown error
			const Poco::Path Path;

			// Auth Token and username (stored in case of reconnection)
			std::string Token;
			std::string Username;

			// a helper that correlates IRC commands to functions
			IRCCorrelator &IRC;

			// private functions
			
			// Connection related functions
			void _connect(long delay=0);

			// message recieve handler
			void _onMessage(const asio::error_code &e, std::size_t size);
			
		public:
			// constructor
			IRCBot(asio::io_context &context, std::string server, std::string portNum,
					IRCCorrelator &IRCCor,
					const Poco::Path filePath);
			// destrcutor
			virtual ~IRCBot();

			// starts the event handle loop
			void start();

			// TODO - move to constructor?
			// Setup functions that must be run before calling start
			// Provides a token to the bot for authentification.
			void giveToken(std::string tok);
			void giveUsername(std::string user);

			// function to write lines to the socket
			void write(const std::string messageString);

			// a friend class to handle correlation (likely to be removed)
			friend IRCCorrelator;
	};
}
#endif
