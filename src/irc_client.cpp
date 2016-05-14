/*
 * Copyright (c) 2015 Devin Carr
 * Licensed under MIT (https://github.com/devincarr/echo/blob/master/LICENSE)
 */

#include "echo/irc_client.h"

IRCClient::IRCClient(Settings* s):
    settings(s),
    _channel(s->channel()),
    irc(IRCSocket()),
    send_queue(new BoundedQueue<std::string>(20)),
    last_sent(0)
    {
        log = spdlog::get("echo");
    }

IRCClient::~IRCClient() {
    disconnect();
    delete send_queue;
}

bool IRCClient::connect(std::string hostname, int port) {
    bool ret = irc.connect(hostname, port);
    std::thread thread(&IRCClient::send_handler,this);
    send_thread = std::move(thread);
    return ret;
}

void IRCClient::disconnect() {
    send_message("PART #" + _channel);
    send_message("QUIT");
    log->debug("Disconnected and sent PART and QUIT");
    irc.disconnect();
    send_queue->push(""); // send a blank message in case
    send_thread.join();
    log->debug("IRC cleaned up");
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
    log->debug("[SENDING] thread started");
    while (connected()) {
        message = send_queue->pull();
        if (!message.empty()) {
            if (ready()) {
                send_message("PRIVMSG #" + _channel + " :" + message);
                log->debug("[SENDING] PRIVMSG #" + _channel + " :" + message);
            } else {
                log->debug("[SENDING] Dropped message: " + message);
            }
        }
    }
    log->debug("[SENDING] thread quit");
}

std::string IRCClient::read() {
    return irc.sread();
}

bool IRCClient::login() {
    if (send_message("PASS " + settings->pass())) {
        if (send_message("NICK " + settings->nick())) {
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
    if (_channel == "#jtv") {
        if (send_message("PRIVMSG #jtv :/w " + settings->owner() + " " + msg)) {
            return true;
        }
    }
    return false;
}

Message IRCClient::parse() {
    std::string buffer(this->read());
    
    // Check watch commands
    if (buffer.substr(0,4).compare("PING") == 0) {
        log->debug("PING recieved");
        send_message("PONG" + buffer.substr(4));
        log->debug("PONG replied");
        return Message("","");
    }

    // Parse text normally
    std::string message;
    std::string user;
    
    // Search for message
    std::string search_str = "PRIVMSG #" + _channel;
    std::size_t fnd = buffer.find(search_str);
    if (fnd != std::string::npos) {
        message = buffer.substr(fnd+search_str.length()+2);
        // remove the trailing \n
        message = message.substr(0, message.length() - 1);
        if (!message.empty()) {
            // Parse user
            fnd = buffer.find("!");
            if (fnd != std::string::npos) {
                user = buffer.substr(1,fnd-1);
                log->debug(user + ": " + message);
                return Message(user,message);
            }
        } else {
            log->debug("EMPTY MESSAGE: " + buffer);
        }
    }
    
    // Bad Message
    return Message("","");
}
