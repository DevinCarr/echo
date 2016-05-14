/*
* Copyright (c) 2015 Devin Carr
* Licensed under MIT (https://github.com/devincarr/echo/blob/master/LICENSE)
*/

#pragma once

#include "echo/irc_client.h"
#include "echo/watcher.h"

class WatcherClient :
    public IRCClient
{
public:
    WatcherClient(Settings* s);
    ~WatcherClient();
    void recv_handler() override;
};

