/*
 * Copyright (c) 2015 Devin Carr
 * Licensed under MIT (https://github.com/devincarr/echo/blob/master/LICENSE)
 */

#include "echo/irc_client.h"
#include "echo/watcher.h"
#include "echo/settings.h"
#include "spdlog/spdlog.h"

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

// Signal catch for Ctrl-C to halt the bot gracefully
void signal_handler(int signal) {
    std::unique_lock<std::mutex> lock(running_mut);
    _running = false;
    running_cv.notify_all();
    spdlog::get("echo")->debug("notified all to quit");
}

void irc_handler(IRCClient* iirc) {
    std::unique_lock<std::mutex> lock(running_mut);
    Watcher* watcher = new Watcher(iirc);
    watcher->start();
    while (_running) running_cv.wait(lock);
    spdlog::get("echo")->debug("irc thread shutting down");
    watcher->stop();
    delete watcher;
}

void whispers_handler(IRCClient* iirc) {
    std::unique_lock<std::mutex> lock(running_mut);
    // Send ready message to owner
    if (iirc->priv_me("Running at: " + iirc->channel())) {
        spdlog::get("echo")->debug("Sent live message to: " + iirc->owner());
    }

    while (_running) running_cv.wait(lock);

    if (iirc->priv_me("Shutting down..")) {
        spdlog::get("echo")->debug("Sent shutdown message to: " + iirc->owner());
    }
}

// Connect to the standard irc server
void connect_to_irc(IRCClient* iirc) {
    auto log = spdlog::get("echo");
    log->debug("Connecting to: " + std::string(tmi_hostname));
    if (iirc->connect(tmi_hostname,port)) {
        log->debug("Connected");
        log->debug("Logging in as: " + iirc->owner());
        if (iirc->login()) {
            log->debug("Logged in");
            log->debug("Joining channel: " + iirc->channel());
            if (iirc->join(iirc->channel())) {
                log->debug("Joined channel");
                std::thread th(irc_handler, iirc);
                irc_thread = std::move(th);
                log->debug("Started watching channel");
            } else {
                log->warn("Failed to join channel " + iirc->channel());
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
    auto log = spdlog::get("echo");
    log->debug("Connecting to: " + std::string(whispers_hostname));
    if (iirc->connect(whispers_hostname,port)) {
        log->debug("Connected");
        log->debug("Logging in as: " + iirc->owner());
        if (iirc->login()) {
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

    // Setup Settings
    Settings* settings = new Settings();
    if (!settings->init()) {
        return 1;
    }

    // Argument parsing
    if (argc > 1) {
        std::cout << "Echo: A twitch.tv bot" << std::endl
        << "To start, put in your twitch username and password in the file "
        << "located here:\n" << settings->settings_filepath << std::endl;
        return 0;
    }

    // Check credentials
    if (!settings->verify_credentials()) {
        std::cout << "Invalid Credentials: Make sure to change them to your twitch username/password\n"
            << "Settings file location: " << settings->settings_filepath << std::endl;
        return 1;
    }

    // Get Logger
    std::shared_ptr<spdlog::logger> log(spdlog::get("echo"));

    _running = true;

    // Begin connections
    IRCClient* irc = new IRCClient(settings);
    IRCClient* wisp = new IRCClient(settings);

    connect_to_irc(irc);
    connect_to_whispers(wisp);

    std::cout << "Running..." << std::endl;
    irc->send("Echo online.");

    // Setup signal watch
    signal(SIGINT, signal_handler);
    
    {
        // Wait here in main loop for the other threads
        std::unique_lock<std::mutex> lock(running_mut);
        while (_running) running_cv.wait(lock);
        irc_thread.join();
        w_thread.join();
        log->debug("Shutting down...");
    }
    std::cout << "Shutting down..." << std::endl;
    
    delete irc;
    delete wisp;
    delete settings;

    log->debug("Shutdown complete");

    return 0;
}

