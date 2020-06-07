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
 */

#include <string>
#include <set>

namespace Twitch
{
	struct token
	{
		std::string accessToken;
		std::string refreshToken;

		std::set<std::string> scopes;
	};
}
