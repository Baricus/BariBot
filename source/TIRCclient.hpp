/* TIRCclient.hpp - Miles Shamo
 *
 * A definition of a C++ class that
 * manages an IRC connection intended
 * to be directed towards Twitch serevers
 */

#include <string>

#include <asio.hpp>
#include <asio/io_context.hpp>

using std::string;

namespace Twitch
{
	class IRCClient
	{
		private:
			static asio::io_context context;
			
			//basic stream socket
			asio::ip::tcp::socket TCPsocket;

		public:
			IRCClient(string server);
			


	};
}
