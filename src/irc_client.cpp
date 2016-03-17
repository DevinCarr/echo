/*
 * Copyright (c) 2015 Devin Carr
 * Licensed under MIT (https://github.com/devincarr/echo/blob/master/LICENSE)
 */

#include "echo/irc_client.h"

IRCClient::IRCClient(Log* l):
    irc(IRCSocket(l)),
    log(l),
    send_queue(new BoundedQueue<std::string>(20)),
    last_sent(0)
    {
        std::thread thread(&IRCClient::send_handler,this);
        send_thread = std::move(thread);
    }

IRCClient::~IRCClient() {
    disconnect();
    delete send_queue;
}

void IRCClient::set_owner(std::string owner) {
    _owner = owner;
}

bool IRCClient::connect(std::string hostname, int port) {
    return irc.connect(hostname, port);
}

void IRCClient::disconnect() {
    send_message("PART #" + _channel);
    send_message("QUIT");
    log->d("Disconnected and sent PART and QUIT");
    irc.disconnect();
    send_queue->push(""); // send a blank message in case
    log->d("sent");
    send_thread.join();
    log->d("cleaned up");
}

// Add a send to a queue to check whether it can send or not.
// A basic check to comply with Twitch IRC rate limiting.
bool IRCClient::send(std::string msg) {
    send_queue->push(msg);
    return true;
}

bool IRCClient::send_message(std::string msg) {
    msg.append("\n");
    if (irc.swrite(msg))
        return true;
    else
        return false;
}

// Check if a message can be sent.
bool IRCClient::ready() {
    clock_t now = clock();
    if (last_sent == 0) {
        last_sent = clock();
        return true;
    }

    float temp_time = (float)(last_sent - now) / CLOCKS_PER_SEC;

    if (temp_time > TIME_BETWEEN_SEND) {
        last_sent = clock();
        return true;
    } else {
        return false;
    }
}

// Message sending queue handler
// Currently a message is dropped if !ready()
void IRCClient::send_handler() {
    std::string message;
    while (connected()) {
        message = send_queue->pull();
        if (!message.empty()) {
            if (!ready()) {
                send_message("PRIVMSG #" + _channel + " :" + message);
                log->d("[SENDING] PRIVMSG #" + _channel + " :" + message);
            } else {
                log->d("[SENDING] Dropped message: " + message);
            }
        }
    }
}

std::string IRCClient::read() {
    return irc.sread();
}

bool IRCClient::login(std::string nick, std::string pass) {
    _nick = nick;
    _pass = pass;

    if (send_message("PASS " + pass)) {
        if (send_message("NICK " + nick)) {
            return true;
        }
    }

    return false;
}

bool IRCClient::join(std::string channel) {
    _channel = channel;

    if (send_message("JOIN #" + channel)) {
        return true;
    }

    return false;
}

bool IRCClient::priv_me(std::string msg) {
    if (!_nick.empty() && !_channel.empty()) {
        if (send_message("PRIVMSG #jtv :/w " + _owner + " " + msg)) {
            return true;
        }
    }

    return false;
}

Message IRCClient::parse() {
    std::string buffer(this->read());
    
    // Check watch commands
    if (buffer.substr(0,4).compare("PING") == 0) {
        log->d("PING recieved");
        send_message("PONG" + buffer.substr(4));
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
