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

#include <string>
#include <vector>
#include <map>

// for smatch
#include <regex>

#include <asio.hpp>

// analysis of IRC commands
#include "IRCCorrelator.hpp"
#include "IRCResults.hpp"

using std::string;
using std::vector;

namespace Twitch
{
	class IRCBot
	{
		private:
			// Socket and other network objects
		
			// io context refference
			asio::io_context &Context;

			// connection data
			string Server, PortNumber;

			// strand for handle execution (currently unneeded)
			asio::strand<asio::io_context::executor_type> _Strand;

			// resolver
			asio::ip::tcp::resolver IPresolver;

			//basic stream socket Pointer
			asio::ip::tcp::socket TCPsocket;

			//ASIO error system
			asio::error_code error;

			//read buffer
			string inString;
			asio::dynamic_string_buffer
			<string::value_type, string::traits_type, string::allocator_type> inBuffer;

			// All non-network related members

			// Auth Token and username (stored in case of reconnection)
			string Token;
			string Username;

			// a helper that correlates IRC commands to functions
			IRCCorrelator &IRC;

			// private functions
			
			// Connection related functions
			void _connect(long delay=0);
			void _authenticate();

			// message recieve handler
			void _onMessage(const asio::error_code &e, std::size_t size);

			
		public:
			// constructor
			IRCBot(asio::io_context &context, string server, string portNum,
					IRCCorrelator &IRCCor);

			// starts the event handle loop
			void start();

			// Setup functions that must be run before calling start
			// Provides a token to the bot for authentification.
			void giveToken(string tok);
			void giveUsername(string user);
	};
}
#endif
