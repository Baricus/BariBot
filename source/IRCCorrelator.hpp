/* IRCCorrelator.hpp - Miles Shamo
 *
 * This defines a very simple structure (in concept),
 * the IRCCorrelator, that matches an IRC command
 * to a function which processes and returns
 * all data needed to handle the command, such
 * as what needs to be sent in return, any
 * state changes, etc.  As all function prototypes
 * must be identical, all use the same "results"
 * struct (defined in it's own hpp file) to return
 * information by reference, without needing an
 * arbitrary list of parameters to do so; we get
 * an arbitrarily sized structure instead which is
 * only syntactically nicer so it's implementation
 * is rather important.  
 *
 * To ensure the function works, it returns true on
 * succesful completion and false otherwise.
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

#ifndef IRC_CORRELATOR
#define IRC_CORRELATOR

#include "IRCResults.hpp"

#include <map>		//...map
#include <string>	//...string
#include <regex> 	//smatch

namespace Twitch
{
	class IRCCorrelator
	{
		public:
			std::map<std::string, bool (*)(std::smatch &, Twitch::IRCResults &)> SFM;

			IRCCorrelator(); // constructor to populate map
	};
}
#endif
