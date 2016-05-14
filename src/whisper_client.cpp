/*
* Copyright (c) 2015 Devin Carr
* Licensed under MIT (https://github.com/devincarr/echo/blob/master/LICENSE)
*/

#include "echo/whisper_client.h"

WhisperClient::WhisperClient(Settings* s): IRCClient(s)
{ }

WhisperClient::~WhisperClient()
{ }

void WhisperClient::recv_handler() {
    // Send ready message to owner
    if (priv(settings->owner(), "Running at: " + settings->channel())) {
        spdlog::get("echo")->debug("Sent live message to: " + settings->owner());
    }

    settings->wait();

    if (priv(settings->owner(), "Shutting down..")) {
        spdlog::get("echo")->debug("Sent shutdown message to: " + settings->owner());
    }
}

bool WhisperClient::priv(std::string user, std::string msg) {
    if (send_message("PRIVMSG #jtv :/w " + user + " " + msg)) {
        return true;
    }
    return false;
}
