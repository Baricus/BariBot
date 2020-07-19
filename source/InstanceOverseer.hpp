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
#include "CommandCorrelator.hpp"

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
			// the token folder that those tokens are in
			Poco::Path TokenPath;

			// stored client list
			std::vector<Poco::File> StoredClients;

			// Running client list
			std::vector<Twitch::IRCBot *> Clients;

			// a path to a folder of clients (each a folder)
			Poco::Path ClientPath;

			// client ID and secret
			std::string ClientID, ClientSecret;

			// helpers to pass to ALL instances (unmodified) to handle each IRC command
			// and user command
			IRCCorrelator MasterIRCCorrelator;
			CommandCorrelator MasterCommandCorrelator;
	
			// a function to renew those tokens as needed (to pass to clients)
			bool _renewToken(Twitch::token &);

			// a function to spawn threads from to run the IO context
			void _runContext();

			// strings containing the server and port to connect to
			const std::string Server = "irc.chat.twitch.tv";
			const std::string Port = "6667";

			// path to credentials
			Poco::Path CredsPath;

		public:			
			// constructor, destructor
			Overseer();
			virtual ~Overseer();

			// function to set ClientID and Client Secret
			void setAppCreds(std::string ID, std::string secret);

			// on to generate a new client instance
			bool createClient(const std::string &name, const Poco::File &token);

			// function to create a new instance from a token.
			void launchClientInstance(Poco::File &tokenSelected, std::string server, std::string port);

			// a function to create a token file
			void createToken(std::istream &in, Poco::Path &dir);

			// a function to delete a token file and update the list of tokens accordingly
			void deleteToken(int index);

			// function to startup baribot
			void init();

			// client management IO loop
			void run();
	};
}

#endif
