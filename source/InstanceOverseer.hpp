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


#include <map>

#include "token.hpp"
#include "TIRCBot.hpp"

namespace Twitch
{
	class Overseer
	{
		private:
			// a map of tokens to IRCclients to enable easy correlation
			std::map<Twitch::token, Twitch::IRCBot> clients;

			// client ID and secret
			std::string ClientID, ClientSecret;			
	
			// a function to renew those tokens as needed (to pass to clients)
			bool _renewToken(Twitch::token &);

		public:
			
	};
}
