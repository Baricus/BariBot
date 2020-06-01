/* TIRCBot.hpp - Miles Shamo
 *
 * A definition of a C++ class that
 * manages an IRC connection intended
 * to be directed towards Twitch serevers
 */

#include <asio/buffer.hpp>
#include <asio/io_context.hpp>
#include <string>
#include <vector>

#include <asio.hpp>


using std::string;
using std::vector;

namespace Twitch
{
	class IRCBot
	{
		private:
			// connection data
			string Server, PortNumber;

			asio::strand<asio::io_context::executor_type> _Strand;

			// resolver
			asio::ip::tcp::resolver IPresolver;

			//basic stream socket
			asio::ip::tcp::socket TCPsocket;

			//ASIO error system
			asio::error_code error;

			//read buffer
			string inString;
			asio::dynamic_string_buffer
			<string::value_type, string::traits_type, string::allocator_type> inBuffer;


			// private functions
			void _connect(string serv, string portNum);
			void _start();
			void _onMessage(const asio::error_code &e, std::size_t size);

		public:
			IRCBot(asio::io_context &context, string server, string portNum);


			


	};
}
