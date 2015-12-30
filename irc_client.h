/*
 * The MIT License (MIT) Copyright (c) 2015 Devin Carr
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 */

#ifndef _irc_client_h_
#define _irc_client_h_

#include "irc_socket.h"
#include "logger.h"

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
    bool connect(char * hostname, int port);
    void disconnect();
    bool send(std::string msg);
    std::string read();
    bool login(std::string nick, std::string pass);
    bool join(std::string channel);
    bool priv_me(std::string msg);
    void parse();
};

#endif
