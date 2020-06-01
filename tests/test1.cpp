
#include "catch.hpp"

#include <asio.hpp>
#include <asio/io_context.hpp>
#include "../source/TIRCBot.hpp"

SCENARIO("Creating individual clients")
{
	asio::io_context io;

	GIVEN("A single IRCBot cleint")
	{
		Twitch::IRCBot client(io, "192.168.1.76", "23");

		WHEN("We start the io service the bot listens for return pings")
		{
			io.run();
		}
	}
}
