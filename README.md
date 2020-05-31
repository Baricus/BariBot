# BariBot
BariBot will be a twitch specific IRC-bot written from the socket level up in C++.  It is being developed on Debian GNU/Linux 10 (buster) but is, in theory, compatible with other distros.  At the moment, a windows variant is not planned, as this will likely require a rewrite of the network code.  

### Current Status
BariBot is, at the moment, little more than a concept.  Minimal (to no) code exists.  As the project matures this file will be updated to reflect the capabilities of the program.

## Development Roadmap
- [X] Create a repo (one thing on this list had to be simple)
- [ ] Create a TCP specific socket wrapper class
- [ ] Implemenent an IRC client class using above wrapper
- [ ] Adapt client to properly authenticate with Twitch servers and maintain connection
- [ ] Add Twitch specific IRC features

## License
[GNU AGPLv3](https://choosealicense.com/licenses/agpl-3.0/)
