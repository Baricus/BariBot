/* InstanceOverseer.hpp - Miles Shamo
 *
 * This class is designed to create and destroy
 * instances of the TIRCBot class as well as any
 * other helper classes needed to communicate with
 * Twitch's servers.    
 *
 * As part of this process, the Overseer manages
 * tokens, storing the program's client ID and secret
 * and any active tokens, to allow for renewal when
 * required.
 *
 */

#ifndef TWITCH_OVERSEER
#define TWITCH_OVERSEER

#include <asio/executor_work_guard.hpp>
#include <map>
#include <vector>
#include <utility>

#include <Poco/File.h>

#include "token.hpp"
#include "TIRCBot.hpp"

#include "IRCCorrelator.hpp"

namespace Twitch
{
	class Overseer
	{
		private:
			// the universal IO context and work guard
			asio::io_context Context;
			asio::executor_work_guard<asio::io_context::executor_type> work;

			// a vector of threads to work on that context
			std::vector<std::thread> Threads;

			// a vector of Token files
			std::vector<Poco::File> TokenFiles;

			// a map of token files to IRCclients to enable easy correlation
			std::vector<Twitch::IRCBot *> Clients;

			// client ID and secret
			std::string ClientID, ClientSecret;

			// a helper to pass to ALL instances (unmodified) to handle each IRC command
			IRCCorrelator MasterIRCCorrelator;
	
			// a function to renew those tokens as needed (to pass to clients)
			bool _renewToken(Twitch::token &);

		public:			
			// constructor, destructor
			Overseer();
			virtual ~Overseer();

			// function to set ClientID and Client Secret
			void setAppCreds(std::string ID, std::string secret);

			// function to create a new instance from a token.
			// Returns a pair for easy insertion into the map
			//
			// May become a private function or change return type to just the token
			void createClientInstance(int tokenSelected, std::string server, std::string port);

			// a function to create a token file
			void createToken(std::istream &in, Poco::File &dir);

			// a function to delete a token file and update the list of tokens accordingly
			void deleteToken(int index);

			// function to start the overseer
			void run();
	};
}

#endif
