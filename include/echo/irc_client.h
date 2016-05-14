/*
 * Copyright (c) 2015 Devin Carr
 * Licensed under MIT (https://github.com/devincarr/echo/blob/master/LICENSE)
 */

#pragma once

#include "echo/bounded_queue.h"
#include "echo/irc_socket.h"
#include "echo/message.h"
#include "echo/settings.h"
#include "spdlog/spdlog.h"

#include <ctime>
#include <memory>
#include <string>
#include <thread>

#define TIME_BETWEEN_SEND 20

class IRCClient {
protected:
    Settings* settings;
    std::string _channel;
    IRCSocket irc;
    std::shared_ptr<spdlog::logger> log;

    BoundedQueue<std::string>* send_queue;
    std::thread send_thread;
    std::thread recv_thread;
    clock_t last_sent;

    bool ready();
    bool send_message(std::string msg);  

public:
    IRCClient(Settings* s);
    ~IRCClient();

    bool connected() { return irc.connected(); }
    std::string channel() { return _channel; }
    std::string owner() { return settings->owner(); }

    bool connect(std::string hostname, std::string channel, int port = 6667);
    void disconnect();
    bool send(std::string msg);
    std::string read();
    bool login();
    bool join(std::string channel);
    Message parse();

    // Abstract functions
    virtual void send_handler();
    virtual void recv_handler() = 0;
};
