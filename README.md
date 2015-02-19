# echo
A Twitch.tv IRC echo bot.

## Usage
Echo will listen in on the twitch.tv chat and join in on the spam when it begins. It usually waits for a 1/4 ratio of normal chat to spammed messages to repeat and participate. The bot will also wait up to 10 seconds from when it joins the spam to speak again to make sure not to get too out of control.

## Instructions
```shell
$ python kappa.py
```
Type in your password using the oauth key generated from [here](http://twitchapps.com/tmi/) and then the channel of the twitch.tv stream.

## License
MIT - (view LICENSE for more information)
