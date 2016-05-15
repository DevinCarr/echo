/*
 * Copyright (c) 2015 Devin Carr
 * Licensed under MIT (https://github.com/devincarr/echo/blob/master/LICENSE)
 */

#include "echo/irc_client.h"

IRCClient::IRCClient(Settings* s):
    settings(s),
    irc(IRCSocket()),
    log(spdlog::get("echo")),
    send_queue(new BoundedQueue<std::string>(20)),
    last_sent(0)
    { }

IRCClient::~IRCClient() {
    disconnect();
    delete send_queue;
}

// Connect/Login/Join channel of the IRC hostname
bool IRCClient::connect(std::string hostname, std::string channel, int port) {
    if (!irc.connect(hostname, port)) {
        log->warn("Failed to join channel " + settings->channel());
        return false;
    }
    log->debug("Connected");
    log->debug("Logging in as: " + settings->owner());

    if (!login()) {
        log->warn("Failed to login");
        return false;
    }  
    log->debug("Logged in");
    log->debug("Joining channel: " + channel);

    if (!join(channel)) {
        log->warn("Failed to connect to " + hostname);
        return false;
    }
    log->debug("Joined channel");

    // Send extra commands
    //send_message("CAP REQ :twitch.tv/tags");
    send_message("CAP REQ :twitch.tv/membership");
    //send_message("CAP REQ :twitch.tv/commands");

    // Set up recv_thread
    recv_thread = std::thread(&IRCClient::recv_handler, this);
    // Set up send_thread
    send_thread = std::thread(&IRCClient::send_handler, this);
    log->debug("Started watching channel");

    return true;
}

void IRCClient::disconnect() {
    if (!irc.connected()) {
        if (send_thread.joinable())
            send_thread.join();
        if (recv_thread.joinable())
            recv_thread.join();
        return;
    }
    send_message("PART #" + _channel);
    send_message("QUIT");
    log->debug("Disconnected and sent PART and QUIT");
    irc.disconnect();
    send_queue->push(""); // send a blank message in case
    send_thread.join();
    recv_thread.join();
    log->debug("IRC cleaned up");
}

// Add a send to a queue to check whether it can send or not.
// A basic check to comply with Twitch IRC rate limiting.
bool IRCClient::send(std::string msg) {
    send_queue->push(msg);
    return true;
}

// Private send function to append a newline character to a message before sending
bool IRCClient::send_message(std::string msg) {
    log->debug("MSG: " + msg);
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

    float temp_time = (float)(now - last_sent) / CLOCKS_PER_SEC;

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
                log->debug("[SENDING] Dropped message (sending to frequent): " + message);
            }
        }
    }
    log->debug("[SENDING] thread quit");
}

// Read from the IRC socket
std::string IRCClient::read() {
    return irc.sread();
}

// Login to IRC
bool IRCClient::login() {
    if (send_message("PASS " + settings->pass())) {
        if (send_message("NICK " + settings->nick())) {
            return true;
        }
    }

    return false;
}

// Join a IRC channel
bool IRCClient::join(std::string channel) {
    _channel = channel;

    if (send_message("JOIN #" + channel)) {
        return true;
    }

    return false;
}

// Parse a message from the IRC channel socket and parse it into a Message struct
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
