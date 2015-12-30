# echo
A Twitch.tv IRC echo bot.

## Usage
*THIS CURRENTLY NOT READY:* The bot can only currently connect to channels and the group twitch chat for whispers.
Echo will listen in on the twitch.tv chat and join in on the spam when it begins. It usually waits for a 1/4 ratio of normal chat to spammed messages to repeat and participate. The bot will also wait up to 10 seconds from when it joins the spam to speak again to make sure not to get too out of control.

## Build
```shell
$ make
```
## Running
Example:

```shell
$ ./echo -p oauth:asdasdasdasdasdasdasd -n twitch_username -c twitch_channel
```
Get your password using the oauth key generated from [here](http://twitchapps.com/tmi/) and then the channel of the twitch.tv stream.

## Logs
The bot wil create a log file (currently stored in the current directory of the executable).

The log files will match: `YYYY-MM-DD.log`

## Help
```shell
$ ./echo -h
```

```shell
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
