/* InstanceOverseer.cpp - Miles Shamo
 *
 * This is the implementation file for the
 * Overseer class, which manages all instances
 * that communicate with Twitch in the system.
 *
 */


#include "InstanceOverseer.hpp"

// Poco headers for a HTTPS client session to send
// POSTS to Twitch servers
#include <Poco/Net/HTTPAuthenticationParams.h>
#include <Poco/Net/HTTPSClientSession.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>

#include <Poco/URI.h>

#include <iostream>


/* setContext
 *
 * setContext turns on and off the IO context.  
 * It creates X number threads of when applied.
 *
 * If threads is zero, it turns off the context
 *
 * It returns true assuming everything happens
 *
 * TODO - To be implemented
 */
bool Twitch::Overseer::setContext(int count)
{
	Context.run();

	return true;
}

/* setAppCreds
 *
 * a simple setter function for the ClientID and clientSecret
 */
void Twitch::Overseer::setAppCreds(std::string id, std::string secret)
{
	ClientID = id;
	ClientSecret = secret;
}

/* _renewToken
 *
 * renewToken takes a token and replaces it with
 * a new token through POSTS to id.twitch.tv.  
 *
 * TODO Modify to renew both standard and client
 * tokens (client tokens used for twitch api calls)
 * 		- this may not be needed
 */
bool Twitch::Overseer::_renewToken(Twitch::token &tok)
{
	// create's a URI (hardcoded)
	Poco::URI renew("https://id.twitch.tv/oauth2/token--data-urlencode"
					"?grant_type=refresh_token"
					"&refresh_token=" + tok.refreshToken +
					"&client_id=" + ClientID +
					"&client_secret=" + ClientSecret);

	// creates the https connection and the proper request
	Poco::Net::HTTPSClientSession session(renew.getHost(), renew.getPort());
	Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_POST, renew.getPath());

	Poco::Net::HTTPResponse response;
	session.sendRequest(request);
	session.receiveResponse(response);
	response.write(std::cout);

	// if we recieved JSON back (an OK request)
	if (response.getStatus() == Poco::Net::HTTPResponse::HTTP_OK)
	{


		return true;
	}

	// otherwise, the request failed
	return false;
}


/* createClientInstance
 *
 * createInstance creates a new TwitchIRC client instance
 * and binds it to the current token to return.  
 *
 * If the token is invalid, the client throws an authentication error
 * which is caught here.  This prompts a token renewal and a
 * retry.
 */
std::pair<Twitch::token, Twitch::IRCBot *> Twitch::Overseer::createClientInstance(
		Twitch::token tok, 
		std::string server,
		std::string portN
		)
{
	Twitch::IRCBot *client = new Twitch::IRCBot(Context, server, portN);

	// TODO temporary fix - just refresh the token
	_renewToken(tok);

	// authenticates the client
	client->giveToken(tok.accessToken);
	client->giveUsername(tok.username);

	// TODO - try catch for token renew
	client->start();
	
	// have a working cleint and token so return
	return std::make_pair(tok, client);
}
