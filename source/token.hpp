/* token.hpp - Miles Shamo
 *
 * This is a simple struct designed
 * to hold a token and all related
 * data.  
 *
 * This avoids throwing JSON anywhere
 * but where it's needed to be originally
 * parsed.
 *
 * As Twitch suggests, we ignore refresh times
 * and simply use a token until it stops working.  
 * At that point, we simply use the refresh to
 * generate a new valid token. 
 *
 * For simplicity, we bundle the username alongside
 * the token for easy refference
 */

#ifndef TWITCH_TOKEN
#define TWITCH_TOKEN

#include <string>
#include <set>
#include <ostream>
#include <istream>

namespace Twitch
{
	struct token
	{
		std::string username;

		std::string accessToken;
		std::string refreshToken;

		std::string scopes;
	};
}

// two overloaded operators
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
#endif
