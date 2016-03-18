# echo [![Build Status](https://travis-ci.org/DevinCarr/echo.svg?branch=master)](https://travis-ci.org/DevinCarr/echo?branch=master)
A Twitch.tv IRC echo bot.

## Usage
*THIS CURRENTLY NOT READY:* The bot can only currently connect to channels and the group twitch chat for whispers.
Echo will listen in on the twitch.tv chat and join in on the spam when it begins. It usually waits for a 1/4 ratio of normal chat to spammed messages to repeat and participate. The bot will also wait up to 10 seconds from when it joins the spam to speak again to make sure not to get too out of control.

## Build
Building `echo` requires CMake
```shell
mkdir build
cd build
cmake ..
make
```

## Building Tests
```shell
mkdir build           # build directory
cd build
cmake -Dtest=on ..    # build with tests linked
make
make unittest             # or ./echo-test
```

## Running
Example:

```shell
./echo -p oauth:asdasdasdasdasdasdasd -n twitch_username -c twitch_channel
```
Get your password using the oauth key generated from [here](http://twitchapps.com/tmi/) and then the channel of the twitch.tv stream.

## Logs
The bot wil create a log file (currently stored in the current working directory in `logs/`).
*Note*: You do have to `mkdir logs` the logs folder for now.

The log files will match: `YYYY-MM-DD.log`

## Help
```shell
./echo -h

Echo: A twitch.tv bot
-p  oauth password for twitch.tv account.
-n  twitch.tv username.
-c  twitch.tv channel for echo to join
```

## Other
Echo occassionally shows:
```shell
< PING
> PONG :tmi.twitch.tv
```
This is a common response that echo makes to make sure that the twitch IRC doesn't disconnect echo for being AFK. ([Further](https://github.com/justintv/Twitch-API/blob/master/IRC.md))

## License
MIT - (view LICENSE for more information)
