/* IRCCorrelator.hpp - Miles Shamo
 *
 * This defines a very simple structure (in concept),
 * the IRCCorrelator, that matches an IRC command
 * to a function which processes and, through the magic
 * of friend classes, queues up any necessary returns,
 * logging, etc.  
 *
 * To ensure the function worked, it returns true on
 * succesful completion and false if any error occured.
 *
 * If necessary, the map can be made private and access
 * to the required member functions (find, [], etc) can
 * be made available in implementation.  At present,
 * with only one coder working and only one set use,
 * this is not needed to prevent misuse.  However, I will
 * strive to ensure that this can always be done later
 * (I won't alter the map outside of this class).
 *
 * If any functions require storing data, private members
 * can be created and used within them.
 */

#ifndef TWITCH_IRC_CORRELATOR
#define TWITCH_IRC_CORRELATOR

#include <map>		//...map
#include <string>	//...string
#include <regex> 	//smatch
#include <functional>

namespace Twitch
{
	// forward declaration of IRCBot
	class IRCBot;

	class IRCCorrelator
	{
		public:
			std::map<std::string, std::function<std::string(std::smatch &, Twitch::IRCBot *Caller)>> SFM;

			IRCCorrelator(); // constructor to populate map
	};
}
#endif
