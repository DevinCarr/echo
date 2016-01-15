/*
 * Copyright (c) 2015 Devin Carr
 * Licensed under MIT (https://github.com/devincarr/echo/blob/master/LICENSE)
 */

#pragma once

#include "logger.h"
#include "irc_client.h"
#include "message.h"
#include "bounded_queue.h"

#include <cmath>
#include <iostream>
#include <functional>
#include <pthread.h>
#include <string>
#include <thread>
#include <vector>

// The total number of msgs to check in a thread
#define QUEUE_BOUND 10
// The difference in length tolerance for the message text
#define DIFF_TOL 3
// Maximum number of parse threads running at a time
#define MAX_THREADS 5

class Watcher {
private:
    Log* log;
    IRCClient* irc;
    volatile bool _running;
    std::vector<std::thread> _running_threads;
    BoundedQueue<Message>* msg_queue;
public:
    Watcher(Log* l, IRCClient* i);
    ~Watcher();
    bool running() { return _running; }
    void start();
    void stop();

    void push_handler();
    void pull_handler();
    void parse_handler(std::vector<Message> msgs);
};

