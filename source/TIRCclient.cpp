/* TIRCclient.cpp - Miles Shamo
 *
 * Implementation of the twitch-specific
 * IRC client
 */

#include "TIRCclient.hpp"
#include <asio/buffered_read_stream.hpp>
#include <asio/connect.hpp>

// static io_context declaration
asio::io_context Twitch::IRCClient::context;


// Constructor
// handles connection
//
// socket is created via initialization list
Twitch::IRCClient::IRCClient(string server) : TCPsocket(context)
{
	// DNS lookup
	asio::ip::tcp::resolver IPresolver(context);
	auto endpoints = IPresolver.resolve(server, "IRC");

	// attempts to connect
	asio::connect(TCPsocket, endpoints);


}
