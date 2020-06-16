/* IRCCorrelator.cpp - Miles Shamo
 *
 * This is the implementation file for
 * the IRCCorrelator, a glorified map
 * of IRC command strings to functions
 * handling those commands.  
 *
 * This file mainly implements the
 * constructor, where the map is
 * populated with each command.
 *
 * Each command is a lambda expression
 * with a matching argument signature.
 *
 * All return true if successful and false otherwise;
 * other necessary data is returned through the 
 * IRCResults struct in the function signature.
 *
 */

#include "IRCCorrelator.hpp"
#include "IRCResults.hpp"


// the constructor which populates the map
Twitch::IRCCorrelator::IRCCorrelator()
{


	// NOTICE
	//
	//
	SFM["NOTICE"] = [](std::smatch &Sm, Twitch::IRCResults &Res) -> bool
	{
		// temporary function to test viability
		Res.output = "Hello World";
		Res.shouldOutput = true;

		return true;
	};

}
