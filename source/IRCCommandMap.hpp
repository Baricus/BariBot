/* IRCCommandMap.hpp - Miles Shamo
 *
 * This file simply declares a map of
 * strings to function pointers, which
 * is defined in IRCCommandMap.cpp.
 *
 * This is only intended for use in
 * implementation files
 */

#ifndef IRC_COMMAND_MAP
#define IRC_COMMAND_MAP

#include <map>

#include <string>
#include <regex>

namespace Twitch
{
	extern std::map<std::string, bool (*)(std::smatch &)> IRCCommandHandlerMap;
}
#endif
