/* token.cpp - miles shamo
 *
 * A quick file to implement 
 * stream operators for the
 * token struct for very
 * simple serializing for
 * file storage
 *
 */

#include "token.hpp"

// two overloaded operators to enable stream in and out
std::ostream& operator<<(std::ostream& os, const Twitch::token &T)
{
	os   << T.username     << std::endl
		 << T.accessToken  << std::endl
		 << T.refreshToken << std::endl
		 << T.scopes       << std::endl;

	return os;
}

std::istream& operator>>(std::istream& is, Twitch::token &T)
{
	is >> T.username;
	is >> T.accessToken;
	is >> T.refreshToken;
	is >> T.scopes;

	return is;
}
