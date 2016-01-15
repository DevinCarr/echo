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
#include <thread>

// Global variables for threads and state monitoring
volatile bool running = false;
Log* logger;
std::thread irc_thread;
std::thread w_thread;

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

void irc_handler(IRCClient* iirc) {
    Watcher* watcher = new Watcher(logger,iirc);
    watcher->start();
    while (iirc->connected() && running) {
        ;;
    }
    watcher->stop();
    delete watcher;
    pthread_exit(0);
}

void whispers_handler(IRCClient* iirc) {
    // Send ready message to owner
    if (iirc->priv_me("Running at: " + channel)) {
        logger->d("Sent live message to: " + iirc->get_owner());
    }

    while(iirc->connected() && running) {
       ;;
    }

    if (iirc->priv_me("Shutting down..")) {
        logger->d("Sent shutdown message to: " + iirc->get_owner());
    }
}

// Connect to the standard irc server
void connect_to_irc(IRCClient* iirc) {
    iirc->set_owner(owner);
    logger->d("Connecting to: " + std::string(tmi_hostname));
    if (iirc->connect(tmi_hostname,port)) {
        logger->d("Connected");
        logger->d("Logging in as: " + nick);
        if (iirc->login(nick,pass)) {
            logger->d("Logged in");
            logger->d("Joining channel: " + channel);
            if (iirc->join(channel)) {
                logger->d("Joined channel");
                std::thread th(irc_handler, iirc);
                irc_thread = std::move(th);
                logger->d("Started watching channel");
            } else {
                logger->e("Failed to join channel " + channel);
            }
        } else {
            logger->e("Failed to login");
        }
    } else {
        logger->e("Failed to connect to " + std::string(tmi_hostname));
    } 
}

// Connect to the group chat server for whispers
void connect_to_whispers(IRCClient* iirc) {
    iirc->set_owner(owner);
    logger->d("Connecting to: " + std::string(whispers_hostname));
    if (iirc->connect(whispers_hostname,port)) {
        logger->d("Connected");
        logger->d("Logging in as: " + nick);
        if (iirc->login(nick,pass)) {
            logger->d("Logged in");
            logger->i("Joining channel: jtv");
            if (iirc->join("#jtv")) {
                logger->d("Joined channel");
                std::thread th(whispers_handler, iirc);
                w_thread = std::move(th);
                logger->d("Started watching channel");
            } else {
                logger->e("Failed to join channel #jtv");
            }
        } else {
            logger->e("Failed to login");
        }
    } else {
        logger->e("Failed to connect to " + std::string(whispers_hostname));
    } 
}

// Begin shutting down and cleanup
void close() {
    logger->i("Shutting down...");
    if (!running) {
        logger->d("Joining irc_thread and w_thread");
        if (irc_thread.joinable()) irc_thread.join();
        if (w_thread.joinable()) w_thread.join();
        logger->d("Main threads joined");
    } else {
        logger->w("Running state is still true");
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

