/*
 * Copyright (c) 2015 Devin Carr
 * Licensed under MIT (https://github.com/devincarr/echo/blob/master/LICENSE)
 */

#include "echo/watcher.h"

Watcher::Watcher(Log* l, IRCClient* i):
    log(l),
    irc(i),
    msg_queue(new BoundedQueue<Message>(QUEUE_BOUND * 2)),
    _running(false)
    { }

Watcher::~Watcher() {
    if (_running) {
        stop();
    }
    delete msg_queue;
}

void Watcher::start() {
    log->d("Starting watcher threads");
    _running = true;

    // Start the push thread
    std::thread push_t(&Watcher::push_handler,this);
    _running_threads.push_back(std::move(push_t));

    // Start the pull threads
    for (int i = 0; i < MAX_THREADS; i++) {
        std::thread pull_t(&Watcher::pull_handler,this);
        _running_threads.push_back(std::move(pull_t));
    }

    log->d("Watcher threads running");
}

void Watcher::stop() {
    // Set the stop for the push/pull threads
    log->d("Halting " + std::to_string(_running_threads.size()) + " running threads");
    _running = false;

    for (auto& t: _running_threads) {
        // Adds exit messages just if there are no messages in the queue
        msg_queue->push(Message("",""));
    }

    for (auto& thread: _running_threads) {
        thread.join();
    }
    
    log->d("All watcher threads joined");
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
        parse_messages(std::vector<Message>(temp));
        temp.clear();
    }
}

// Parse the messages from the queue for repeats
void Watcher::parse_messages(std::vector<Message> msgs) {
    int size = msgs.size();
    log->d("Parsing initialized, msgs.size() = " + std::to_string(size));
    for (int i = 0; i < size; i++) {
        int index_length = msgs[i].text.length();
        for (int j = i + 1; j < size; j++) {
            if (std::abs((double)(msgs[j].text.length() - index_length)) < DIFF_TOL) {
                std::string score = longest_common_substr(msgs[i].text,msgs[j].text);
                if (!score.empty()) {
                    log->d("String Match: " + score + " messages: " + msgs[i].text + " ~ " +  msgs[j].text);
                    break;
                }
            }
        }
    }
}

// Longest common substring calculation (https://en.wikipedia.org/wiki/Longest_common_substring_problem)
std::string longest_common_substr(std::string S, std::string T) {
    size_t z = 0; // length of LCSS
    std::string ret; // LCSS return value
    std::vector<std::vector<char>> L(S.length(), std::vector<char>(T.length()));

    for (size_t i = 0; i < S.length(); i++) {
        for (size_t j = 0; j < T.length(); j++) {
            if (S[i] == T[j]) {
                if (i == 0 or j == 0)
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
    if (z < (size_t)std::ceil(0.6*std::max(S.length(),T.length())))
        ret = "";
    else 
        return ret;
}


