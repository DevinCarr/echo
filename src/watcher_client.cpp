/*
* Copyright (c) 2015 Devin Carr
* Licensed under MIT (https://github.com/devincarr/echo/blob/master/LICENSE)
*/

#include "echo/watcher_client.h"

WatcherClient::WatcherClient(Settings* s): IRCClient(s)
{ }

WatcherClient::~WatcherClient()
{ }

void WatcherClient::recv_handler() {
    Watcher* watcher = new Watcher(this);
    watcher->start();
    settings->wait();
    spdlog::get("echo")->debug("irc thread shutting down");
    watcher->stop();
    delete watcher;
}
