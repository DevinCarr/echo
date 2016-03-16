/*
 * Copyright (c) 2015 Devin Carr
 * Licensed under MIT (https://github.com/devincarr/echo/blob/master/LICENSE)
 */

#pragma once

#include "echo/logger.h"
#include "echo/irc_client.h"
#include "echo/message.h"
#include "echo/bounded_queue.h"

#include <atomic>
#include <cmath>
#include <condition_variable>
#include <iostream>
#include <functional>
#include <pthread.h>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

// The total number of msgs to check in a thread
#define QUEUE_BOUND 20
// The difference in length tolerance for the message text
#define DIFF_TOL 3
// Maximum number of parse threads running at a time
#define MAX_THREADS 5

class Watcher {
private:
    Log* log;
    IRCClient* irc;
    std::atomic_bool _running;
    std::vector<std::thread> _running_threads;
    BoundedQueue<Message>* msg_queue;
public:
    Watcher(Log* l, IRCClient* i);
    ~Watcher();
    bool running() { return _running && irc->connected(); }
    void start();
    void stop();

    void push_handler();
    void pull_handler();
    void parse_messages(std::vector<Message> msgs);

    std::mutex queue_pull_mutex;
};

std::string longest_common_substr(std::string S, std::string T);
