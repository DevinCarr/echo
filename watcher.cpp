#include "watcher.h"

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

/*
 * Thread handler functions, and ld_calc
 */
static void* push_handler(void* args);
static void* pull_handler(void* args);
static void* parse_handler(void* args);
int levenshtein_distance(std::string s, std::string t);

void Watcher::start() {
    log->d("Starting watcher threads");
    _push_thread = pthread_t();
    _pull_thread = pthread_t();
    _running = true;
    if (pthread_create(&_push_thread, NULL, push_handler, this) == 0) {
        log->w("Failed to create a push_handler thread!");
    }

    if (pthread_create(&_pull_thread, NULL, pull_handler, this) == 0) {
        log->w("Failed to create a pull_handler thread!");
    }
    log->d("Watcher threads running");
}

void Watcher::stop() {
    // Join threads running in the _parse_threads
    int ret;
    void* status;
    for (int i = 0; i < _parse_threads.size(); i++) {
        ret = pthread_join(_parse_threads[i], &status);
        if (ret) {
            log->w("Error joining parse thread after stopping; err id:" + ret);
        }
    }
    
    // Set the stop for the push/pull threads
    _running = false;
    ret = pthread_join(_push_thread, &status);
    if (ret) {
        log->w("Error joining push thread after stopping; err id:" + ret);
    }

    ret = pthread_join(_pull_thread, &status);
    if (ret) {
        log->w("Error joining pull thread after stopping; err id:" + ret);
    }
    
}

// Creates a new thread and marks it as in use in the _parse_threads vector
// If there are already too many threads running, return NULL
bool Watcher::get_thread(pthread_t t) {
    if (_parse_threads.size() >= MAX_THREADS) {
        _parse_threads.push_back(t);
        return true;
    } else {
        return false;
    }
}

// Remove a completed thread from the _parse_threads
void Watcher::free_thread(pthread_t tid) {
    auto thread = _parse_threads.begin();
    while(thread != _parse_threads.end()) {
        if (pthread_equal(tid,*thread) != 0) {
            _parse_threads.erase(thread);
        } else {
            thread++;
        }
    }
}

// Push thread to acquire the messages and push them into
// the queue to be pulled out with the pull_handler
static void* push_handler(void* args) {
    Watcher* watcher = (Watcher*)args;
    BoundedQueue<Message>* queue = watcher->get_queue();
    IRCClient* iirc = watcher->get_irc();

    // Main push loop
    while(iirc->connected() && watcher->running()) {
        queue->push(iirc->parse());
    }
    
    // Thread exit
    pthread_exit(0);
}

// Pull thread that will pull the messages from the queue and
// build a new parse_handler to compare the messages
static void* pull_handler(void* args) {
    Watcher* watcher = (Watcher*)args;
    Log* log = watcher->get_log();
    IRCClient* iirc = watcher->get_irc();
    BoundedQueue<Message>* queue = watcher->get_queue();
    ThreadArgs* t_args = new ThreadArgs();
    t_args->w = watcher;
    std::vector<Message> temp;

    // Main pull loop
    while(iirc->connected() && watcher->running()) {
        temp.push_back(queue->pull());
        if (temp.size() >= QUEUE_BOUND) {
            // start thread 
            pthread_t thread;
            if (watcher->get_thread(thread)) {
                t_args->q = new std::vector<Message>(temp);
                if (pthread_create(&thread, NULL, parse_handler, t_args) == 0) {
                    log->w("Failed to create a parse_handler thread!");
                } else {
                    log->w("No free threads available, dropping " + std::to_string(temp.size()) + " messages!");
                }
            }
            temp.empty();
        }
    }
    
    // Thread exit
    pthread_exit(0);
}

// Parse the messages from the queue for repeats
static void* parse_handler(void* args) {
    Watcher* watcher = ((struct ThreadArgs*)args)->w;
    Log* log = watcher->get_log();
    std::vector<Message>* msgs = ((struct ThreadArgs*)args)->q;
    int size = msgs->size();
    for (int i = 0; i < size; i++) {
        int index_length = (*msgs)[i].text.length();
        for (int j = i; j < size; j++) {
            if (abs((*msgs)[j].text.length() - index_length) < DIFF_TOL) {
                int score = levenshtein_distance((*msgs)[i].text,(*msgs)[j].text);
                if (score < DIFF_TOL) {
                    log->d("LD Score: " + std::to_string(score) + " messages: " + (*msgs)[i].text + " ~ " +  (*msgs)[j].text);
                }
            }
        }
    }
    // Close up thread
    delete msgs;
    watcher->free_thread(pthread_self());
    pthread_exit(0); 
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

