/*
 * Copyright (c) 2015 Devin Carr
 * Licensed under MIT (https://github.com/devincarr/echo/blob/master/LICENSE)
 */

#ifndef _irc_client_h_
#define _irc_client_h_

#include "irc_socket.h"
#include "logger.h"
#include "message.h"

class IRCClient {
private:
    std::string _owner;
    std::string _nick;
    std::string _pass;
    std::string _channel;

    IRCSocket irc;
    Log* log;

public:
    IRCClient(Log* l);
    ~IRCClient();
    bool connected() { return irc.connected(); }
    void set_owner(std::string owner);
    std::string get_owner() { return _owner; }
    bool connect(char * hostname, int port);
    void disconnect();
    bool send(std::string msg);
    std::string read();
    bool login(std::string nick, std::string pass);
    bool join(std::string channel);
    bool priv_me(std::string msg);
    Message parse();
};

#endif
