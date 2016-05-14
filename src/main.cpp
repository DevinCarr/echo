/*
 * Copyright (c) 2015 Devin Carr
 * Licensed under MIT (https://github.com/devincarr/echo/blob/master/LICENSE)
 */

#include "echo/watcher_client.h"
#include "echo/whisper_client.h"
#include "echo/settings.h"
#include "spdlog/spdlog.h"

#include <iostream>

std::string tmi_hostname = "irc.twitch.tv";
std::string whispers_hostname = "irc.chat.twitch.tv";

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

    settings->start();

    // Begin connections
    WatcherClient* irc_wat = new WatcherClient(settings);
    irc_wat->connect(tmi_hostname,settings->channel());
    WhisperClient* irc_wsp = new WhisperClient(settings);
    irc_wsp->connect(whispers_hostname,"jtv");

    std::cout << "Running..." << std::endl;
    std::cout << "(Type q + <enter> to quit)" << std::endl;
    irc_wat->send("Echo online.");

    // Wait for input
    while (std::cin.get() != 'q') {}

    std::cout << "Shutting down..." << std::endl;

    // Close the other threads
    settings->stop();
    log->debug("Shutting down...");
    irc_wat->disconnect();
    irc_wsp->disconnect();

    // Clean up
    delete irc_wat;
    delete irc_wsp;
    delete settings;

    log->debug("Shutdown complete");

    return 0;
}

