/*
 * Copyright (c) 2015 Devin Carr
 * Licensed under MIT (https://github.com/devincarr/echo/blob/master/LICENSE)
 */

#include "logger.h"
#include "irc_client.h"
#include "watcher.h"

#include <atomic>
#include <condition_variable>
#include <iostream>
#include <signal.h>
#include <thread>

// Global variables for threads and state monitoring
std::atomic_bool _running;
std::mutex running_mut;
std::condition_variable running_cv;
Log* logger;
std::thread irc_thread;
std::thread w_thread;

std::string tmi_hostname = "irc.twitch.tv";
std::string whispers_hostname = "192.16.64.180";
int port = 6667;
std::string owner = "biogeneration";
std::string pass = "";
std::string nick = "";
std::string channel = "";

// Signal catch for Ctrl-C to halt the bot gracefully
void signal_handler(int signal) {
    std::unique_lock<std::mutex> lock(running_mut);
    _running = false;
    running_cv.notify_all();
    logger->d("notified all to quit");
}

void irc_handler(IRCClient* iirc) {
    std::unique_lock<std::mutex> lock(running_mut);
    Watcher* watcher = new Watcher(logger,iirc);
    watcher->start();
    running_cv.wait(lock, [](){ return !_running; });
    logger->d("irc thread shutting down");
    watcher->stop();
    delete watcher;
}

void whispers_handler(IRCClient* iirc) {
    std::unique_lock<std::mutex> lock(running_mut);
    // Send ready message to owner
    if (iirc->priv_me("Running at: " + channel)) {
        logger->d("Sent live message to: " + iirc->get_owner());
    }

    running_cv.wait(lock, [](){ return !_running; });

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

    _running = true;

    // Begin connections
    IRCClient* irc = new IRCClient(logger);
    IRCClient* wisp = new IRCClient(logger);

    connect_to_irc(irc);
    connect_to_whispers(wisp);

    // Setup signal watch
    signal(SIGINT, signal_handler);
    
    {
        // Wait here in main loop for the other threads
        std::unique_lock<std::mutex> lock(running_mut);
        running_cv.wait(lock, [](){ return !_running; });
        irc_thread.join();
        w_thread.join();
        logger->i("Shutting down...");
    }
    
    delete irc;
    delete wisp;

    logger->i("Shutdown complete");
    logger->close_file();

    delete logger;

    return 0;
}

