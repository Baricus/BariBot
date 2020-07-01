# BariBot
BariBot will be a twitch specific IRC-bot written in C++.  It is being developed on Debian GNU/Linux 10 (buster) but is, in theory, compatible with other distros and (hopefully) Windows.  However, it is not being compiled or tested on any other platforms at this time.  
BariBot uses the [ASIO](https://think-async.com/Asio/index.html) socket library (non-boost) to handle socket connections in a platform independant fashion.  While not yet implemented, to handle HTTP requests, BariBot uses [POCO](pocoproject.org).  This is needed to renew tokens and access any other API calls which are not part of the IRC framework (for example getting user statistics). 

### Current Status
BariBot currently has a rudimentary interface that allows for file storage of necessary information (tokens and client keys) in order to then launch client instances.  When tokens expire, the program detects and renews them.  At present, work is focused on reaching the point where user interaction through Twitch chat is possible.  

## Development Roadmap
- [X] Create a repo (one thing on this list had to be simple)
- [X] Install ASIO
- [X] Install POCO
- [ ] Implemenent an IRC client class using ASIO tcp sockets
  - [X] Open a TCP socket to a given server+port
  - [X] Handle connection failures with fallof retries
  - [X] Parse incoming IRC commands
  - [ ] Process IRC commands
  - [X] Generate outbound IRC commands
  - [ ] Connect to the requested IRC channels
  - [X] Properly authenticate with Twitch servers
  - [X] Maintain connection with Twitch servers (ping pong)
  - [ ] Handle user commands
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
