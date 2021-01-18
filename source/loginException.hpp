/* loginException.hpp - Miles Shamo
 *
 * This is a quick exception to
 * be thrown on issues with login,
 * extending the runtime error
 * to include info on the calling
 * instance to find and replace it.
 *
 */

#include <stdexcept>

#include <Poco/Path.h>

namespace Twitch
{

	class IRCBot;

	struct loginException : std::runtime_error
	{
		const Twitch::IRCBot *Caller;
		const Poco::Path ClientPath;

		loginException(const char* message, const Twitch::IRCBot *Caller, const Poco::Path ClientPath) : runtime_error(message), ClientPath(ClientPath)
		{
			this->Caller = Caller;
		}
	};
}
