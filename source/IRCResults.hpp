/* IRCResults.hpp - Miles Shamo
 *
 * This is a definition of a "result"
 * structore for the IRCCorrelator object.
 *
 * This object is used to simplify the function
 * heading, containing all necessary returned information
 * for any IRCCorrelator function.  
 *
 * This drastically simplifies the function signature,
 * but only kicks the can down the road.  This struct
 * needs every necessary return value for the functions
 * and how to differentiate between them (should I output, etc).
 *
 * Therefore, this is intentionally limited to as few
 * items as possible, to prevent a ridiculous structure.
 */

#ifndef IRC_RESULTS
#define IRC_RESULTS

#include <string>

namespace Twitch
{
	struct IRCResults
	{
		// all booleans about what to "do" after this command
		
		bool shouldOutput;  // should the bot output a string

		bool shouldClose;   // should the bot close the socket

		std::string output; // a string to output if needed
	};
}
#endif
