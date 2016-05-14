/*
 * Copyright (c) 2015 Devin Carr
 * Licensed under MIT (https://github.com/devincarr/echo/blob/master/LICENSE)
 */

#include "echo/watcher.h"

Watcher::Watcher(IRCClient* i):
    irc(i),
    _running(false),
    msg_queue(new BoundedQueue<Message>(QUEUE_BOUND * 2))
    { 
        log = spdlog::get("echo");
    }

Watcher::~Watcher() {
    if (_running) {
        stop();
    }
    delete msg_queue;
}

void Watcher::start() {
    log->debug("Starting watcher threads");
    _running = true;

    // Start the push thread
    std::thread push_t(&Watcher::push_handler,this);
    _running_threads.push_back(std::move(push_t));

    // Start the pull threads
    for (int i = 0; i < MAX_THREADS; i++) {
        std::thread pull_t(&Watcher::pull_handler,this);
        _running_threads.push_back(std::move(pull_t));
    }

    log->debug("Watcher threads running");
}

void Watcher::stop() {
    // Set the stop for the push/pull threads
    log->debug("Halting " + std::to_string(_running_threads.size()) + " running threads");
    _running = false;

    for (int i = 0; i < _running_threads.size(); i++) {
        // Adds exit messages just if there are no messages in the queue
        msg_queue->push(Message("",""));
    }

    for (auto& thread: _running_threads) {
        thread.join();
    }
    
    log->debug("All watcher threads joined");
}

// Push thread to acquire the messages and push them into
// the queue to be pulled out with the pull_handler
void Watcher::push_handler() {
    // Main push loop
    Message temp;
    while (running()) {
        temp = irc->parse();
        if (!temp.bad()) {
            msg_queue->push(temp);
        }
    }
}

// Pull thread that will pull the messages from the queue and
// build a new handler to compare the messages
void Watcher::pull_handler() {
    std::vector<Message> temp;
    Message pulled;
    // get thread id
    std::ostringstream ss;
    ss << std::this_thread::get_id();
    std::string thread_id = ss.str();

    // Check for access
    while (running()) {
        { // lock guard scope
            std::unique_lock<std::mutex> lock(queue_pull_mutex);

            // Pull loop
            while (running() && (temp.size() < QUEUE_BOUND)) {
                pulled = msg_queue->pull();
                // check if the message is empty, notifying end
                if (!pulled.bad()) {
                    temp.push_back(pulled);
                }
            }
        } // end guard scope
        parse_messages(std::vector<Message>(temp), thread_id);
        temp.clear();
    }
}

// Parse the messages from the queue for repeats
void Watcher::parse_messages(std::vector<Message> msgs, std::string thread_id) {
    int size = msgs.size();
    std::map<std::string,size_t> store_map;
    log->debug("[PARSING] Initialized with size " + std::to_string(size) + " on thread: " + thread_id);
    for (int i = 0; i < size; i++) {
        int index_length = msgs[i].text.length();
        for (int j = i + 1; j < size; j++) {
            if (std::abs((double)(msgs[j].text.length() - index_length)) < DIFF_TOL) {
                std::string message = longest_common_substr(msgs[i].text,msgs[j].text);
                if (!message.empty()) {
                    log->debug("[PARSING] String Match: " + message + " messages: " + msgs[i].text + " ~ " +  msgs[j].text);
                    auto search = store_map.find(message);
                    if (search != store_map.end()) {
                        search->second++;
                    } else {
                        store_map[message] = 1;
                    }
                }
            }
        }
    }

    // check for amount of messages to count as valid
    std::string msg;
    size_t max_count = 0;
    for (auto kv: store_map) {
        if (kv.second > max_count) {
            max_count = kv.second;
            msg = kv.first;
        }
    }
    if (max_count > (size_t)std::floor(0.3*QUEUE_BOUND)) {
        if (!msg.empty()) {
            log->debug("[PARSING] Valid message: " + msg);
            irc->send(msg);
        }
    }
}

// Longest common substring calculation (https://en.wikipedia.org/wiki/Longest_common_substring_problem)
std::string longest_common_substr(std::string S, std::string T) {
    size_t z = 0; // length of LCSS
    std::string ret; // LCSS return value
    std::vector<std::vector<size_t>> L(S.length(), std::vector<size_t>(T.length(),0));

    for (size_t i = 0; i < S.length(); i++) {
        for (size_t j = 0; j < T.length(); j++) {
            if (S[i] == T[j]) {
                if (i == 0 || j == 0)
                    L[i][j] = 1;
                else
                    L[i][j] = L[i-1][j-1] + 1;
                if (L[i][j] >= z) {
                    z = L[i][j];
                    ret = S.substr(i-z+1,z);
                }
            } else {
                L[i][j] = 0;
            }
        }
    }

    // check if the length of the longest substring is around 60%
    // of the max(S,T) lengths
    if (z < (size_t)std::ceil(0.6 * std::max(S.length(),T.length())))
        ret = "";

    return ret;
}


