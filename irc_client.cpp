/*
 * Copyright (c) 2015 Devin Carr
 * Licensed under MIT (https://github.com/devincarr/echo/blob/master/LICENSE)
 */

#include "irc_client.h"

IRCClient::IRCClient(Log* l): log(l), irc(IRCSocket(l)) { }

IRCClient::~IRCClient() {
    disconnect();
}

void IRCClient::set_owner(std::string owner) {
    _owner = owner;
}

bool IRCClient::connect(char * hostname, int port) {
    return irc.connect(hostname, port);
}

void IRCClient::disconnect() {
    return irc.disconnect();
}

bool IRCClient::send(std::string msg) {
    msg.append("\n");
    return irc.swrite(msg);
}

std::string IRCClient::read() {
    std::string buffer = irc.sread();
    return buffer;
}

bool IRCClient::login(std::string nick, std::string pass) {
    _nick = nick;
    _pass = pass;

    if (send("PASS " + pass)) {
        if (send("NICK " + nick)) {
            return true;
        }
    }

    return false;
}

bool IRCClient::join(std::string channel) {
    _channel = channel;

    if (send("JOIN #" + channel)) {
        return true;
    }

    return false;
}

bool IRCClient::priv_me(std::string msg) {
    if (!_nick.empty() && !_channel.empty()) {
        if (send("PRIVMSG #jtv :/w " + _owner + " " + msg)) {
            return true;
        }
    }

    return false;
}

Message IRCClient::parse() {
    std::string buffer;
    buffer = read();
    
    // Check watch commands
    if (buffer.substr(0,4).compare("PING") == 0) {
        log->d("PING recieved");
        send("PONG" + buffer.substr(4));
        log->d("PONG replied");
        return Message("irc","ping");
    }

    // Parse text normally
    std::string message;
    std::string user;
    
    // Search for message
    std::string search_str = "PRIVMSG #" + _channel;
    std::size_t fnd = buffer.find(search_str);
    if (fnd != std::string::npos) {
        message = buffer.substr(fnd+search_str.length()+2);
        if (!message.empty()) {
            // Parse user
            fnd = buffer.find("!");
            if (fnd != std::string::npos) {
                user = buffer.substr(1,fnd-1);
                log->d(user + ": " + message);
                return Message(user,message);
            }
        }
    }
    
    // Bad Message
    return Message("","");
}
