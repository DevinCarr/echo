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

#include "irc_socket.h"

const int MAX_MESSAGE = 4096;

IRCSocket::IRCSocket(Log* l): _connected(false), log(l) {
    if ((_sockfd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
        log->e("Socket create failure.");
        exit(-1);
    }

    int on = 1;
    if (setsockopt(_sockfd, SOL_SOCKET, SO_REUSEADDR, (char const*)&on, sizeof(on)) == -1) {
        log->e("Invalid socket.");
        exit(-1);
    }
    fcntl(_sockfd, F_SETFL, O_NONBLOCK);
    fcntl(_sockfd, F_SETFL, O_ASYNC);
}

IRCSocket::~IRCSocket() {
    disconnect();
}

bool IRCSocket::connect(char * hostname, int port) {
    hostent* he;

    if (!(he = gethostbyname(hostname))) {
        log->e("Could not resolve hostname");
        exit(-1);
    }

    sockaddr_in addr;

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr = *((const in_addr*)he->h_addr);
    memset(&(addr.sin_zero), '\0', 8);

    if (::connect(_sockfd, (sockaddr*)&addr, sizeof(addr)) == -1) {
        log->e("Could not connect to hostname: " + std::string(hostname));
        close(_sockfd);
        exit(-1);
    }

    _connected = true;
    return _connected;
}

void IRCSocket::disconnect() {
    if (_connected) {
        close(_sockfd);
        _connected = false;
    }
}

std::string IRCSocket::sread() {
    char buffer[MAX_MESSAGE];

    memset(buffer, 0, MAX_MESSAGE);

    int res = recv(_sockfd, buffer, MAX_MESSAGE - 1, 0);
    if (res > 0) {
        return std::string(buffer);
    } else {
        log->e("Bad sread(): disconnecting");
        disconnect();
    }

    return "";
}

bool IRCSocket::swrite(std::string msg) {
    if (_connected) {
        if (send(_sockfd, msg.c_str(), strlen(msg.c_str()), 0) == -1) {
            return false;
        }
    }

    return true;
}