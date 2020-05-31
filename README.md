# BariBot
BariBot will be a twitch specific IRC-bot written from the ground up in C++.  It is being developed on Debian GNU/Linux 10 (buster) but is, in theory, compatible with other distros and (hopefully) Windows.  However, it is not being compiled or tested on any other platforms at this time.  
BariBot uses the [ASIO](https://think-async.com/Asio/index.html) socket library (non-boost) to handle socket connections in a platform independant fashion.  

### Current Status
BariBot is, at the moment, little more than a concept.  Minimal (to no) code exists.  As the project matures this file will be updated to reflect the capabilities of the program.

## Development Roadmap
- [X] Create a repo (one thing on this list had to be simple)
- [ ] Implemenent an IRC client class using ASIO tcp sockets
- [ ] Adapt client to properly authenticate with Twitch servers and maintain connection
- [ ] Add Twitch specific IRC features

## License
[GNU AGPLv3](https://choosealicense.com/licenses/agpl-3.0/)
