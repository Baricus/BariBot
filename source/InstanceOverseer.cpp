/* InstanceOverseer.cpp - Miles Shamo
 *
 * This is the implementation file for the
 * Overseer class, which manages all instances
 * that communicate with Twitch in the system.
 *
 */


#include "InstanceOverseer.hpp"
#include "TIRCBot.hpp"

// Poco headers for a HTTPS client session to send
// POSTS to Twitch servers
#include <Poco/Dynamic/Struct.h>
#include <Poco/Net/HTTPAuthenticationParams.h>
#include <Poco/Net/HTTPSClientSession.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>

// Util headers for making a URI, copying streams, and parsing JSON
#include <Poco/StreamCopier.h>
#include <Poco/URI.h>
#include <Poco/JSON/Parser.h>

// headers for handling files and folders
#include <Poco/Path.h>
#include <Poco/File.h>

#include <asio/executor_work_guard.hpp>
#include <cstdlib>
#include <iostream>
#include <istream>
#include <fstream>
#include <stdexcept>
#include <thread>


/* Overseer (Constructor)
 *
 * starts a IO work to prevent stoppage
 *
 */
Twitch::Overseer::Overseer() : work(make_work_guard(Context))
{
	
}


/* ~Overseer (destructor)
 *
 * stops IO service and joins threads
 *
 */
Twitch::Overseer::~Overseer()
{
	Context.stop();

	for (auto &T : Threads)
	{
		if (T.joinable())
			T.join();
	}

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
	Poco::URI renew("https://id.twitch.tv/oauth2/token"
					"?grant_type=refresh_token"
					"&refresh_token=" + tok.refreshToken +
					"&client_id=" + ClientID +
					"&client_secret=" + ClientSecret);


	// creates the https connection and the proper request
	Poco::Net::HTTPSClientSession session(renew.getHost(), renew.getPort());
	Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_POST, renew.toString(), Poco::Net::HTTPMessage::HTTP_1_1);


	// catches the response header and raw output
	Poco::Net::HTTPResponse response;
	session.sendRequest(request);
	// output comes as a stream
	std::istream& outputRecieved = session.receiveResponse(response);


	// if we recieved JSON back (an OK request)
	if (response.getStatus() == Poco::Net::HTTPResponse::HTTP_OK)
	{
		// convert output to stringstream to JSON
		std::stringstream rawJSON;
		Poco::StreamCopier::copyStream(outputRecieved, rawJSON);

		// parse to a dynamic "Var" and extract to an "Object"
		Poco::JSON::Parser parser;
		auto result = parser.parse(rawJSON);
		auto JSONObject = result.extract<Poco::JSON::Object::Ptr>();

		// extract relevant data from JSON and update token
		tok.accessToken = JSONObject->get("access_token").toString();
		tok.refreshToken = JSONObject->get("refresh_token").toString();

		return true;
	}

	// otherwise, the request failed
	// TODO - Log
	return false;
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



/* run
 *
 * run loads in all pre-existing tokens, etc
 * and then starts an I/O loop to allow for
 * the launching and stopping of client
 * instances and token management.
 *
 */
void Twitch::Overseer::run()
{
	using std::cout;
	using std::cin;
	using std::endl;

	cout << "***Initializing BariBot***" << endl;
	cout << "Searching for stored tokens..." << endl;

	// generate path to tokens
	Poco::Path TokenPath(false);
	TokenPath.pushDirectory(".Tokens");

	// search every token in the folder
	Poco::File TokenFolder(TokenPath);
	if (!TokenFolder.exists())
	{
		cout << "No Token Directory found, creating" << endl;
		TokenFolder.createDirectory();
	}
	else
	{
		// check to ensure this is a directory
		if (!TokenFolder.isDirectory())
			throw std::runtime_error("A file with the same name as the token folder exists");

		// get all files in folder
		TokenFolder.list(TokenFiles);

		cout << "Found " << TokenFiles.size() << " token files" << endl;
	}

	// get's a number of threads
	cout << "Please input a number of threads to work IO: " << endl << "> ";
	int threadCount;
	cin >> threadCount;
	
	for (int i = 0; i < threadCount; i++)
	{
		// runs context with new threads
		Threads.push_back(std::thread( [this, i]
				{
					cout << "Thread " << i << " activated." << endl;

					try
					{
						Context.run();		
					}
					catch(const std::runtime_error &e)
					{
						cout << "Caught runtime error: " << e.what();
					}
				}));
	}


	cout << "Starting I/O loop" << endl << endl;
	

	cout << "Welcome to BariBot" << endl;


	// a lambda to print tokens
	auto printTokens = [&](std::string request, bool shouldPrompt=false) -> int
	{
		// check to ensure we have values
		if (TokenFiles.size() == 0)
		{
			cout << "ERROR: No tokens" << endl;
			return -1;
		}

		cout << request << endl;
		for (int i = 0; i < TokenFiles.size(); i++)
			cout << "\t" << (i+1) << " - " << 
				Poco::Path(TokenFiles[i].path()).getFileName() << endl;

		if (shouldPrompt)
		{
			cout << "> ";
			int input;
			cin >> input;

			// error check
			if (input > TokenFiles.size() || input < 1)
			{
				cout << "ERROR: out of bounds" << endl;
				return -1;
			}

			return input-1;
		}
		else
		{
			return 0;
		}
	};


	int menuChoice, selection;
	do
	{
		// prompt (for now, very simplistic)
		cout << "***Please select an option***"    << endl
			 << "\t0 - Exit"                       << endl
			 << endl
			 << "\t1 - List all Tokens"            << endl
			 << "\t2 - Add a new token"            << endl
			 << "\t3 - Delete an existing token"   << endl
			 << endl
			 << "\t4 - Create new client instance" << endl
			 << "\t5 - Stop client instance"       << endl;
		cout << "> ";
		cin >> menuChoice;

		// execute based on instructions
		switch(menuChoice)
		{
			case 1:
				printTokens("Current tokens:");	
				break;
			case 2:
				createToken(cin, TokenFolder);
				break;

			case 3:
				if ((selection = printTokens("Choose a token to delete:", true)) == -1)
					break;

				deleteToken(selection);
				break;

			case 4:
				if ((selection = printTokens("Choose a token to use:", true)) == -1)
					break;

				createClientInstance(selection, "irc.chat.twitch.tv", "6667");
				break;

			case 5:
				
				break;

			default:
				// do nothing
				break;
		}

	}while(menuChoice != 0);
}


/* _createToken
 *
 * This prompts the user to create a new token file
 * from an instream, with the established token directory
 * passed to it.  The token is written to a file and added
 * to the list of eligable tokens
 *
 */
void Twitch::Overseer::createToken(std::istream &in, Poco::File &dir)
{
	using std::cout;
	using std::endl;

	std::string fileName, username, token, refresh, scopes;

	// name
	cout << "Creating a token"      << endl
		 << "Enter a fileName: ";
	in >> fileName;

	// creates new file
	Poco::Path path(dir.path(), fileName + ".tok");
	Poco::File newToken(path);

	if (newToken.exists())
	{
		cout << "ERROR: Token file already exists" << endl;
		return;
	}
	else
	{
		newToken.createFile();
		cout << "Succesfully created token file" << endl;
	}

	// user
	cout << "Enter the login username: ";
	in >> username;

	// token
	cout << "Enter the Oauth2 token: ";
	in >> token;

	// refresh
	cout << "Enter the refresh token: ";
	in >> refresh;

	// scopes
	cout << "Enter the list of scopes: ";
	in >> scopes;

	// create the token
	Twitch::token temp;
	temp.username = username;
	temp.accessToken = token;
	temp.refreshToken = refresh;
	temp.scopes = scopes;

	// streams in the token
	std::fstream output(newToken.path().c_str());
	output << temp;
	output.close();

	// adds this token to the list of tokens
	TokenFiles.push_back(newToken);
}


/* deleteToken
 *
 * delete token just deletes a token file from
 * the list of stored tokens.  At the same time
 * it actually deletes the file.
 *
 */
void Twitch::Overseer::deleteToken(int index)
{
	// deletes file
	TokenFiles[index].remove();

	// deletes the file in memory
	TokenFiles.erase(TokenFiles.begin() + index);
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
void Twitch::Overseer::createClientInstance(
		int tokenSelected, 
		std::string server,
		std::string port
		)
{
	using std::cout;
	using std::endl;


	// creates a client
	Clients.push_back(Twitch::IRCBot(Context, server, port, MasterIRCCorrelator));

	int curClient = Clients.size() - 1;

	cout << "Reading in selected token" << endl;
	// opens the token file and gets needed data
	std::ifstream tokenF(TokenFiles[tokenSelected].path());

	Twitch::token curTok;
	tokenF >> curTok;
	tokenF.close();

	cout << "Passing token to client" << endl;

	// set's up client
	Clients[curClient].giveToken(curTok.accessToken);
	Clients[curClient].giveUsername(curTok.username);

	// starts the client
	try
	{
		Clients[curClient].start();
	}
	catch(const std::runtime_error &e)
	{
		// deletes client from list

	}

}


