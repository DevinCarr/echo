#ifndef _watcher_h_
#define _watcher_h_

#include "logger.h"
#include "irc_client.h"
#include "message.h"
#include "bounded_queue.h"

#include <cmath>
#include <iostream>
#include <pthread.h>
#include <string>
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
    volatile bool _running;
    pthread_t _push_thread;
    pthread_t _pull_thread;
    std::vector<pthread_t> _parse_threads;
    BoundedQueue<Message>* msg_queue;
public:
    Watcher(Log* l, IRCClient* i);
    ~Watcher();
    bool running() { return _running; }
    void start();
    void stop();
    IRCClient* get_irc() { return irc; }
    BoundedQueue<Message>* get_queue() { return msg_queue; }
    Log* get_log() { return log; }
    bool get_thread(pthread_t t);
    void free_thread(pthread_t tid);
};

struct ThreadArgs {
    ThreadArgs(): w(nullptr), q(nullptr) { }
    Watcher* w;
    std::vector<Message>* q;
};

#endif
