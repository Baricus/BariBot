# BariBot
BariBot will be a twitch specific IRC-bot written in C++.  It is being developed on Debian GNU/Linux 10 (buster) but is, in theory, compatible with other distros and (hopefully) Windows.  However, it is not being compiled or tested on any other platforms at this time.  
BariBot uses the [ASIO](https://think-async.com/Asio/index.html) socket library (non-boost) to handle socket connections in a platform independant fashion.  To handle HTTP requests, BariBot uses the [POCO](pocoproject.org) library.  This is needed to renew tokens and make API calls which are not part of the IRC framework (for example getting user statistics). This could in theory be done with ASIO, but is beyond what I was willing to force myself to do and, coupled with additional functionality that a library suite such as POCO provides, it was hard to argue against using it.  Still, I have attempted to keep usage to a minimum out of principle.   

### Current Status
BariBot is, as of the current moment, techincally functional.  It connects to a given channel and parses IRC commands to search for user commands and executes the requested action based on them.  Current work is directed towards managing the bot's backend so that each bot can configure itself independently of all others.  Further work on end user features is planned after this has been accomplished. 

## Development Roadmap
- [X] Create a repo (one thing on this list had to be simple)
- [X] Install ASIO
- [X] Install POCO
- [ ] Implemenent an IRC client class using ASIO tcp sockets
  - [X] Open a TCP socket to a given server+port
  - [X] Handle connection failures with fallof retries
  - [X] Parse incoming IRC commands
  - [X] Process IRC commands (very limited scope)
  - [X] Generate outbound IRC commands
  - [X] Connect to the requested IRC channels
  - [X] Properly authenticate with Twitch servers
  - [X] Maintain connection with Twitch servers (ping pong)
  - [X] Handle user commands
  - [ ] Implement Twitch-specific IRC functionality
- [ ] Create a manager class to handle client instances
  - [X] Create new token files
  - [X] Refresh implicit tokens for instances
  - [ ] Generate client credentials for non IRC api calls 
  - [X] Instantiate instances on demand
  - [ ] Delete instances as needed
  - [ ] Accept live configuration

## License
[GNU AGPLv3](https://choosealicense.com/licenses/agpl-3.0/)
