/*
* Copyright (c) 2015 Devin Carr
* Licensed under MIT (https://github.com/devincarr/echo/blob/master/LICENSE)
*/

#pragma once

#include "echo/irc_client.h"

class WhisperClient :
    public IRCClient
{
private:
    bool priv(std::string user, std::string msg);
public:
    WhisperClient(Settings* s);
    ~WhisperClient();

    void recv_handler() override;
};

