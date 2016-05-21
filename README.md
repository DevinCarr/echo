# echo [![Build Status](https://travis-ci.org/DevinCarr/echo.svg?branch=master)](https://travis-ci.org/DevinCarr/echo?branch=master)
A Twitch.tv IRC echo bot.

## Usage
The bot can only currently connect to a [twitch.tv](https://www.twitch.tv/) chat channel, but could technically connect to any host with some alterations to the message parsing.

Echo will listen in on the twitch.tv chat and look for multiple repeated messages and perform a Least Common Substring calculation to provide a similarity metric. If there are enough messages that are close to the same (and the bot hasn't sent a message recently) it will repeat the message into the chat.

## Acquiring
```shell
$ git clone https://github.com/DevinCarr/echo.git
$ cd echo
$ git submodule update --init --recursive
```

## Running

### Unix/Linux

Requirements:
- CMake >= 2.8
- C++11 compiler:
  - clang++ >= 3.4
  - g++ >= 4.8

```shell
mkdir build
cd build
cmake ..
make
```
Example usage:
```shell
./echo
```

### Windows
Open the solution file in the `echo` folder and build with at least VS2015. (hasn't been tested with other Windows compilers)

### Configuration
After run once, a `settings.xml` file will be created for you to put your twitch username and password to login. The settings file will be located in:
- Windows: `%APPDATA%\Roaming\echo`
- Unix/Linux: `$HOME/.echo`

Get your password using the oauth key generated from [here](http://twitchapps.com/tmi/) and then the channel of the twitch.tv stream.

## Logs
The bot wil create a log file (currently stored in the settings folder (see above) in `logs/`.

## Other
Echo occassionally shows:
```shell
< PING
> PONG :tmi.twitch.tv
```
This is a common response that echo makes to make sure that the twitch IRC doesn't disconnect echo for being AFK. ([Further](https://github.com/justintv/Twitch-API/blob/master/IRC.md))

## Building Tests
```shell
mkdir build           # build directory
cd build
cmake -Dtest=on ..    # build with tests linked
make
make unittest         # or ./echo-test
```

## License
MIT - (view LICENSE for more information)
