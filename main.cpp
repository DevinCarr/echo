/*
 * Copyright (c) 2015 Devin Carr
 * Licensed under MIT (https://github.com/devincarr/echo/blob/master/LICENSE)
 */

#include "logger.h"
#include "irc_client.h"
#include "watcher.h"

#include <iostream>
#include <pthread.h>
#include <signal.h>

// Global variables for threads and state monitoring
volatile bool running = false;
Log* logger;
pthread_t irc_thread;
pthread_t w_thread;

char* tmi_hostname = "irc.twitch.tv";
char* whispers_hostname = "192.16.64.180";
int port = 6667;
std::string owner = "biogeneration";
std::string pass = "";
std::string nick = "";
std::string channel = "";

// Signal catch for Ctrl-C to halt the bot gracefully
void signal_handler(int signal) {
    running = false;
}

void* parse_irc(void* args) {
    IRCClient* iirc = (IRCClient*)args;
    Watcher* watcher = new Watcher(logger,iirc);
    watcher->start();
    while (iirc->connected() && running) {
        ;;
    }
    watcher->stop();
    delete watcher;
    pthread_exit(0);
}

void* parse_whispers(void* args) {
    IRCClient* iirc = (IRCClient*)args;
    
    // Send ready message to owner
    if (iirc->priv_me("Running at: " + channel)) {
        logger->i("Sent live message to: " + iirc->get_owner());
    }

    while(iirc->connected() && running) {
       ;;
    }

    if (iirc->priv_me("Shutting down..")) {
        logger->i("Sent shutdown message to: " + iirc->get_owner());
    }
    pthread_exit(0);
}

// Connect to the standard irc server
void connect_to_irc(IRCClient* iirc) {
    iirc->set_owner(owner);
    logger->i("Connecting to: " + std::string(tmi_hostname));
    if (iirc->connect(tmi_hostname,port)) {
        logger->i("!Connected");
        logger->i("Logging in as: " + nick);
        if (iirc->login(nick,pass)) {
            logger->i("!Logged in");
            logger->i("Joining channel: " + channel);
            if (iirc->join(channel)) {
                logger->i("!Joined channel");
                if (pthread_create(&irc_thread, NULL, parse_irc, iirc) == 0) {
                    logger->i("Started watching channel");
                }
            }
        }
    }
}

// Connect to the group chat server for whispers
void connect_to_whispers(IRCClient* iirc) {
    iirc->set_owner(owner);
    logger->i("Connecting to: " + std::string(whispers_hostname));
    if (iirc->connect(whispers_hostname,port)) {
        logger->i("!Connected");
        logger->i("Logging in as: " + nick);
        if (iirc->login(nick,pass)) {
            logger->i("!Logged in");
            logger->i("Joining channel: jtv");
            if (iirc->join("#jtv")) {
                logger->i("!Joined channel");
                if (pthread_create(&w_thread, NULL, parse_whispers, iirc) == 0) {
                    logger->i("Started watching channel");
                }
            }
        }
    }
}

// Begin shutting down and cleanup
void close() {
    logger->i("Shutting down...");

    void* status;

    if (!running) {
        logger->i("Joining irc_thread");
        int rc = pthread_join(irc_thread, &status);
        if (rc) {
            logger->e("THREAD: irc_thread failed to join");
        }
        logger->d("irc_thread joined");
        logger->d("Joining w_thread");
        rc = pthread_join(w_thread, &status);
        if (rc) {
            logger->e("THREAD: w_thread failed to join");
        }
        logger->d("w_thread joined");
    }
}

int main(int argc, char * argv[]) {
    // Argument parsing
    for (int i=1; i<argc; i+=2) {
        if (strncmp(argv[i], "-p", 2) == 0)
            pass = argv[i+1];
        else if (strncmp(argv[i], "-n", 2) == 0)
            nick = argv[i+1];
        else if (strncmp(argv[i], "-c", 2) == 0)
            channel = argv[i+1];
        else {
            std::cout << "Echo: A twitch.tv bot" << std::endl
                << "-p\toauth password for twitch.tv account." << std::endl
                << "-n\ttwitch.tv username." << std::endl
                << "-c\ttwitch.tv channel for echo to join" << std::endl;
            return 0;
        }
    }

    // Check for args (nickname, password, and channel)
    if (pass.empty() && nick.empty() && channel.empty()) {
        std::cout << "Error: missing pass/nick/channel, see help (-h)" << std::endl;
        return 1;
    }

    // Start logging
    logger = new Log();
    logger->open_file();
    logger->i("Starting echo");

    running = true;

    // Begin connections
    IRCClient* irc = new IRCClient(logger);
    IRCClient* wisp = new IRCClient(logger);

    connect_to_irc(irc);
    connect_to_whispers(wisp);

    // Setup signal watch
    signal(SIGINT, signal_handler);

    // Wait here in main loop for the other threads
    while (irc->connected() && running) {
        ;;
    }
    
    // Begin clean shutdown
    close();

    delete irc;
    delete wisp;

    logger->i("Shutdown complete");
    logger->close_file();

    delete logger;

    return 0;
}

