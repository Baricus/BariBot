/* InstanceOverseer.cpp - Miles Shamo
 *
 * This is the implementation file for the
 * Overseer class, which manages all instances
 * that communicate with Twitch in the system.
 *
 */

#include "InstanceOverseer.hpp"
#include "TIRCBot.hpp"

#include "loginException.hpp"

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
#include <ios>
#include <iostream>
#include <istream>
#include <fstream>
#include <stdexcept>
#include <thread>


/* Overseer (Constructor)
 *
 * starts a IO work to prevent stoppage
 *
 * and sets up filesystem
 *
 * TODO setup configurable directories
 */
Twitch::Overseer::Overseer() : work(make_work_guard(Context))
{
	// path to tokens
	TokenPath = Poco::Path(false);
	TokenPath.pushDirectory(".Tokens");	

	// credentials
	CredsPath = Poco::Path(false);
	CredsPath.pushDirectory(".AppCredentials");

	// clients
	ClientPath = Poco::Path(false);
	ClientPath.pushDirectory(".Clients");
}


/* ~Overseer (destructor)
 *
 * stops IO service and joins threads
 *
 * then deletes every client
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
	
	for (auto C : Clients)
	{
		delete C;
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


/* _runContext
 *
 * runContext is a blocking function
 * to provide a thread for the IOContext
 * to use in asyncronous operations.  It
 * simply runs the context and deals with
 * any errors thrown, until the context is
 * stopped.  
 *
 */
void Twitch::Overseer::_runContext()
{
	while(!Context.stopped())
	{
		try
		{
			Context.run();		
		}
		// handles login exceptions by reneweing the token and re-creating client
		catch(const loginException &e)
		{
			// opens the token (client path + token.tok" and renews
			Poco::Path clientPath = e.ClientPath;
			clientPath.append("token.tok");
			Poco::File expiredToken(clientPath.toString());
			if (!expiredToken.isFile())
			{
				// TODO - log failure in file
				std::cout << "ERROR, WRONG PATH: " << clientPath.toString() << std::endl;
			}
			else
			{
				std::ifstream input(expiredToken.path().c_str());

				if (!input.is_open())
				{
					//TODO - log failed open
				}
				else
				{
					// stream into memory and renew
					Twitch::token temp;
					input >> temp;

					if (!_renewToken(temp))
					{
						//TODO - log failure
					}
					else
					{
						input.close();

						std::ofstream output(expiredToken.path().c_str(), std::ofstream::trunc);

						output << temp;
						output.close();

						// create a new client
						auto client = Poco::File(e.ClientPath);
						launchClientInstance(client, Server, Port);
					}
				}
			}
		}// end catch for login exceptions
	}
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


/* init
 *
 * init loads in credentials, pre-existing token and client data
 * and everything else needed for operation.  It effectively
 * initializes the "state" of the overseer to whatever it was
 * on the previous use.
 *
 */
void Twitch::Overseer::init()
{
	using std::cout;
	using std::cin;
	using std::endl;

	cout << "***Initializing BariBot***" << endl;

	cout << "Searching for stored credentials..." << endl;

	// creates file and grabs data if found
	Poco::File credsFile(CredsPath);

	std::string ID, secret;

	// prompt for new creds
	if (!credsFile.exists())
	{
		cout << "No credentials saved"          << endl
			 << "Please enter the Client ID: ";

		cin >> ID;

		cout << "Please enter the Client secret: ";

		cin >> secret;

		cout << "storing auth for later runs..." << endl;

		credsFile.createFile();
		std::fstream out(credsFile.path().c_str());
		out << ID << endl << secret << endl;
		out.close();
	}
	else
	{
		if (!credsFile.isFile())
			throw std::runtime_error("auth file name taken");

		cout << "Reading in credentials from file..." << endl;

		std::fstream in(credsFile.path().c_str());

		in >> ID >> secret;
	}

	cout << "Setting credentials..." << endl;
	setAppCreds(ID, secret);

	cout << "Searching for stored tokens..." << endl;

	// search every token in the folder
	Poco::File tokenFolder(TokenPath);
	if (!tokenFolder.exists())
	{
		cout << "No token Directory found, creating" << endl;
		tokenFolder.createDirectory();
	}
	else
	{
		// check to ensure this is a directory
		if (!tokenFolder.isDirectory())
			throw std::runtime_error("A file with the same name as the token folder exists");

		// get all files in folder
		tokenFolder.list(TokenFiles);

		cout << "Found " << TokenFiles.size() << " token files" << endl;
	}

	cout << "Searching for stored clients..." << endl;

	// checks if client folder exists and if so, searches for clients within it
	Poco::File clientFolder(ClientPath);
	if (!clientFolder.exists())
	{
		cout << "No client folder found; creating..." << endl;
		clientFolder.createDirectory();
	}
	else if (clientFolder.isFile())
	{
		throw std::runtime_error("A file with the same name as the client folder exists");
	}
	// if we actually have clients, grab them all
	else
	{
		clientFolder.list(StoredClients);

		cout << "Found " << StoredClients.size() << " stored clients" << endl;	
	}

	// get's a number of threads
	cout << "Please input a number of threads to work IO: " << endl << "> ";
	int threadCount;
	cin >> threadCount;
	
	for (int i = 0; i < threadCount; i++)
	{
		// runs context with new threads
		Threads.push_back(std::thread(&Twitch::Overseer::_runContext, this));
	}
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

	// initalizes the overseer
	this->init();

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
			cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
			cin.clear();

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
	
	auto printClients = [&](std::string request, bool shouldPrompt=false) -> int
	{
		// check to ensure we have values
		if (StoredClients.size() == 0)
		{
			cout << "ERROR: No saved clients" << endl;
			return -1;
		}

		cout << request << endl;
		for (int i = 0; i < StoredClients.size(); i++)
			cout << "\t" << (i+1) << " - " << 
				Poco::Path(StoredClients[i].path()).getFileName() << endl;

		if (shouldPrompt)
		{
			cout << "> ";
			int input;
			cin >> input;
			cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
			cin.clear();

			// error check
			if (input > StoredClients.size() || input < 1)
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


	cout << "Starting I/O loop" << endl << endl;
	

	cout << "Welcome to BariBot" << endl;


	int menuChoice, selection;
	do
	{
		// prompt (for now, very simplistic)
		cout  << "***Please select an option***"      << endl
			  << "\t0 - Exit"                         << endl
			  << endl
			  << "\t1 - List all Tokens"              << endl
			  << "\t2 - Add a new token"              << endl
			  << "\t3 - Delete an existing token"     << endl
			  << endl
			  << "\t4 - List all stored Clients"	  << endl
			  << "\t5 - Create a new client Instance" << endl
			  << "\t6 - Launch a client instance"     << endl
			  << "\t7 - Stop client instance"         << endl;
		cout  << "> ";
		cin >> menuChoice;
		cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		cin.clear();

		// execute based on instructions
		switch(menuChoice)
		{
			case 1: // list tokens
				printTokens("Current tokens:");	
				break;
			case 2: // add token
				createToken(cin, TokenPath);
				break;

			case 3: // delete token
				if ((selection = printTokens("Choose a token to delete:", true)) == -1)
					break;

				deleteToken(selection);
				break;

			case 4: // list  clients
				printClients("Stored Clients:");
				break;

			case 5: // create new client
				if ((selection = printTokens("Choose a token to use:", true)) == -1)
					break;

				createClient(cin, TokenFiles[selection]);
				break;

			case 6: // launch client
				if ((selection = printClients("Choose a client to launch:", true)) == -1)
					break;

				launchClientInstance(StoredClients[selection], Server, Port);
				break;

			case 7: // stop client
				

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
void Twitch::Overseer::createToken(std::istream &in, Poco::Path &dir)
{
	using std::cout;
	using std::endl;

	std::string fileName, username, token, refresh, scopes;

	// name
	cout << "Creating a token"      << endl
		 << "Enter a fileName: ";
	in >> fileName;

	// creates new file
	Poco::Path path(dir, fileName + ".tok");
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
	// uses getline to ensure we don't cause issues with multi-word scopes
	in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
	std::getline(in, scopes);
	scopes.pop_back(); // \n

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


/* launchClientInstance
 *
 * creates a new TwitchIRC client instance
 * and binds it to the current token to return.  
 *
 * If the token is invalid, the client throws an authentication error
 * which is caught here.  This prompts a token renewal and a
 * retry.
 *
 * TODO add async connection to ensure that we don't block main thread
 * 			- likely one extra thread dedicated to launching clients
 */
void Twitch::Overseer::launchClientInstance(
		Poco::File &clientFile, 
		std::string server,
		std::string port
		)
{
	// creates a client
	Clients.push_back(
			new Twitch::IRCBot(Context, server, port, 
							   MasterIRCCorrelator, 
							   MasterCommandCorrelator,
							   clientFile.path()));
}


/* createClient
 *
 * generates a new client folder and adds it to the list
 * of launchable clients
 *
 * All clients use the same logs and other files as convention
 * so we create all of them now just in case.
 *
 * The token is a file link, to allow for multiple client instances
 * to share the same token.  This is needed to ensure that updating
 * the token of one client will update the same token for all other
 * clients that use it.  If we do not link these files, the clients
 * will create competing, valid tokens, which if extreme enough can
 * invalidate some of the copies.  We use a hard link to ensure the
 * clients can use the token even after it has been deleted.  
 * 
 */
bool Twitch::Overseer::createClient(std::istream &in, const Poco::File &token)
{
	// gets name from input stream
	std::string name;
	std::cout << "Choose a name for this client: " << std::endl;
	std::getline(in, name);

	// creates a new folder within the clients folder
	Poco::Path newClientPath(ClientPath);
	newClientPath.pushDirectory(name);
	newClientPath.makeDirectory();

	Poco::File newClientFile(newClientPath);

	// existing client/file/etc
	if (newClientFile.exists())
	{		
		return false;
	}

	newClientFile.createDirectory();

	// grabs the path to the folder for later use
	std::string folder = newClientFile.path();

	// creates a link to the provided token file
	token.linkTo(folder + "/token.tok", token.LINK_HARD);

	// creates other files (fall out of scope so auto closed)

	std::ofstream 	log(folder + "/log.txt"), 
				 	disabled(folder + "/disabledCommands.txt"),
					custom(folder + "/customCommands.txt"),
					channels(folder + "/channels.txt");

	// TODO -- ADD headers where applicable

	// add this to the list
	StoredClients.push_back(Poco::File(newClientPath));

	return true;
}
