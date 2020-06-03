# BariBot
BariBot will be a twitch specific IRC-bot written in C++.  It is being developed on Debian GNU/Linux 10 (buster) but is, in theory, compatible with other distros and (hopefully) Windows.  However, it is not being compiled or tested on any other platforms at this time.  
BariBot uses the [ASIO](https://think-async.com/Asio/index.html) socket library (non-boost) to handle socket connections in a platform independant fashion.  
At present, BariBot is tested using the [CATCH2](https://github.com/catchorg/Catch2) testing framework.  

### Current Status
BariBot is, at the moment, little more than a concept.  Minimal (to no) code exists.  As the project matures this file will be updated to reflect the capabilities of the program.

## Development Roadmap
- [X] Create a repo (one thing on this list had to be simple)
- [X] Install ASIO
- [ ] Implemenent an IRC client class using ASIO tcp sockets
  - [X] Open a TCP socket to a given server+port
  - [ ] Parse incoming IRC commands
  - [ ] Generate outbound IRC commands
  - [X] Properly authenticate with Twitch servers
  - [ ] Maintain connection with Twitch servers (ping pong)
  - [ ] Handle user commands
  - [ ] Implement Twitch-specific IRC functionality
- [ ] Create a manager class to handle client instances
  - [ ] Refresh implicit tokens for instances
  - [ ] Generate client credentials for non IRC api calls 
  - [ ] Instantiate instances on demand
  - [ ] Delete instances as needed
  - [ ] Accept live configuration

## License
[GNU AGPLv3](https://choosealicense.com/licenses/agpl-3.0/)
