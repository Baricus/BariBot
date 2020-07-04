/* CommandCorrelator.hpp - Miles Shamo
 *
 * This is a structure almost identical
 * to the IRCCorrelator (defined in 
 * IRCCorrelator.hpp) which matches IRC
 * command strings to the relevant actions
 * to take regarding them.  In the same way,
 * this object matches commands to their
 * specific functions.  
 *
 * The functions have the same headers as
 * IRCCorrelator and are created in the
 * same fashion.  However, this is more
 * flexible than the aformentioned correlator, to
 * hopefully allow dynamic configuration of
 * new commands through existing commands.  
 *
 * As such, we unfortunately have to actually
 * DO SOME WORK and not just directly expose
 * the map of commands.  
 *
 * however, until the above happens, I'm leaving
 * it public to save my head.
 * 
 */

#ifndef TWITCH_COMMAND_CORRELATOR
#define TWITCH_COMMAND_CORRELATOR

#include <set>
#include <map>
#include <regex>
#include <string>

namespace Twitch
{
	class IRCBot;

	class CommandCorrelator
	{
		public:
			// should I use the same approach?	
			std::map<std::string, bool (*)(const std::smatch &, const std::smatch &, Twitch::IRCBot *Caller)> SFM;

		public:
			CommandCorrelator();
	};
}
#endif
