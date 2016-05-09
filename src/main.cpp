/*
 * Copyright (c) 2015 Devin Carr
 * Licensed under MIT (https://github.com/devincarr/echo/blob/master/LICENSE)
 */

#include "echo/irc_client.h"
#include "echo/watcher.h"

#include <atomic>
#include <condition_variable>
#include <iostream>
#include <signal.h>
#include <thread>

// Global variables for threads and state monitoring
std::atomic_bool _running;
std::mutex running_mut;
std::condition_variable running_cv;
std::thread irc_thread;
std::thread w_thread;

std::string tmi_hostname = "irc.twitch.tv";
std::string whispers_hostname = "irc.chat.twitch.tv";
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
    spdlog::get("echo")->info("notified all to quit");
}

void irc_handler(IRCClient* iirc) {
    std::unique_lock<std::mutex> lock(running_mut);
    Watcher* watcher = new Watcher(iirc);
    watcher->start();
    while (_running) running_cv.wait(lock);
    spdlog::get("echo")->info("irc thread shutting down");
    watcher->stop();
    delete watcher;
}

void whispers_handler(IRCClient* iirc) {
    std::unique_lock<std::mutex> lock(running_mut);
    // Send ready message to owner
    if (iirc->priv_me("Running at: " + channel)) {
        spdlog::get("echo")->debug("Sent live message to: " + iirc->get_owner());
    }

    while (_running) running_cv.wait(lock);

    if (iirc->priv_me("Shutting down..")) {
        spdlog::get("echo")->debug("Sent shutdown message to: " + iirc->get_owner());
    }
}

// Connect to the standard irc server
void connect_to_irc(IRCClient* iirc) {
    iirc->set_owner(owner);
    auto log = spdlog::get("echo");
    log->debug("Connecting to: " + std::string(tmi_hostname));
    if (iirc->connect(tmi_hostname,port)) {
        log->debug("Connected");
        log->debug("Logging in as: " + nick);
        if (iirc->login(nick,pass)) {
            log->debug("Logged in");
            log->debug("Joining channel: " + channel);
            if (iirc->join(channel)) {
                log->debug("Joined channel");
                std::thread th(irc_handler, iirc);
                irc_thread = std::move(th);
                log->debug("Started watching channel");
            } else {
                log->warn("Failed to join channel " + channel);
            }
        } else {
            log->warn("Failed to login");
        }
    } else {
        log->warn("Failed to connect to " + std::string(tmi_hostname));
    } 
}

// Connect to the group chat server for whispers
void connect_to_whispers(IRCClient* iirc) {
    iirc->set_owner(owner);
    auto log = spdlog::get("echo");
    log->debug("Connecting to: " + std::string(whispers_hostname));
    if (iirc->connect(whispers_hostname,port)) {
        log->debug("Connected");
        log->debug("Logging in as: " + nick);
        if (iirc->login(nick,pass)) {
            log->debug("Logged in");
            log->debug("Joining channel: jtv");
            if (iirc->join("#jtv")) {
                log->debug("Joined channel");
                std::thread th(whispers_handler, iirc);
                w_thread = std::move(th);
                log->debug("Started watching channel");
            } else {
                log->warn("Failed to join channel #jtv");
            }
        } else {
            log->warn("Failed to login");
        }
    } else {
        log->warn("Failed to connect to " + std::string(whispers_hostname));
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
    std::shared_ptr<spdlog::logger> log;
    try {
        log = spdlog::daily_logger_mt("echo", "logs/echo", 0, 0, true);
        spdlog::set_level(spdlog::level::debug);
        log->info("Starting echo");
    } catch (const spdlog::spdlog_ex& ex) {
        std::cout << "Log failed to initalize." << std::endl
            << "This is probably because the folder \"logs/\" is" << std::endl
            << "not available in the current working directory." << std::endl;
        return 1;
    }

    _running = true;

    // Begin connections
    IRCClient* irc = new IRCClient();
    IRCClient* wisp = new IRCClient();

    connect_to_irc(irc);
    connect_to_whispers(wisp);

    // Setup signal watch
    signal(SIGINT, signal_handler);
    
    {
        // Wait here in main loop for the other threads
        std::unique_lock<std::mutex> lock(running_mut);
        while (_running) running_cv.wait(lock);
        irc_thread.join();
        w_thread.join();
        log->info("Shutting down...");
    }
    
    delete irc;
    delete wisp;

    log->info("Shutdown complete");

    return 0;
}

