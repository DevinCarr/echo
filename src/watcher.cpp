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
    log->d("Parsing initialized, msgs.size(): " + std::to_string(size));
    for (int i = 0; i < size; i++) {
        int index_length = msgs[i].text.length();
        for (int j = i + 1; j < size; j++) {
            // log->d("Parse msg1 size: " + std::to_string(index_length) + " msg2 size: " + std::to_string(msgs[j].text.length()));
            if (std::abs((double)(msgs[j].text.length() - index_length)) < DIFF_TOL) {
                int score = levenshtein_distance(msgs[i].text,msgs[j].text);
                if (score < DIFF_TOL) {
                    log->d("LD Score: " + std::to_string(score) + " messages: " + msgs[i].text + " ~ " +  msgs[j].text);
                    break;
                }
            }
        }
    }
}

// Levenshtein distance calculation (https://en.wikipedia.org/wiki/Levenshtein_distance)
int levenshtein_distance(std::string s, std::string t) {
    if (s == t) return 0;
    if (s.empty()) return t.length();
    if (t.empty()) return s.length();
    
    int t_length = t.length() + 1;
    int v0[t_length];
    int v1[t_length];

    for (int i = 0; i < t_length; i++)
        v0[i] = i;

    for (int i = 0; i < s.length(); i++) {
        v1[0] = i + 1;

        for (int j = 0; j < t_length; j++) {
            int cost = (s[i] == t[j]) ? 0 : 1;
            v1[j + 1] = std::min(v1[j] + 1, std::min(v0[j + 1] + 1, v0[j] + cost));
        }

        for (int j = 0; j < t_length; j++)
            v0[j] = v1[j];
    }

    return v1[t_length];
}

