/*
 * Copyright (c) 2015 Devin Carr
 * Licensed under MIT (https://github.com/devincarr/echo/blob/master/LICENSE)
 */

#include "echo/irc_socket.h"

const int MAX_MESSAGE = 2048;

IRCSocket::IRCSocket(): _connected(false) {
    log = spdlog::get("echo");

    #ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData)) {
        std::cout << "Failed to initialize WSA: " << WSAGetLastError() << std::endl;
    }
    #endif

    if ((_sockfd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
        log->warn("Socket create failure.");
        exit(-1);
    }

    int on = 1;
    if (setsockopt(_sockfd, SOL_SOCKET, SO_REUSEADDR, (char const*)&on, sizeof(on)) == -1) {
        log->warn("Invalid socket.");
        exit(-1);
    }

    #ifdef _WIN32
    u_long m = 0;
    ioctlsocket(_sockfd, FIONBIO, &m);
    #else
    fcntl(_sockfd, F_SETFL, O_NONBLOCK);
    fcntl(_sockfd, F_SETFL, O_ASYNC);
    #endif
}

IRCSocket::~IRCSocket() {
    disconnect();
}

bool IRCSocket::connect(std::string hostname, int port) {
    hostent* he;

    if (!(he = gethostbyname(hostname.c_str()))) {
        log->warn("Could not resolve hostname");
        #ifdef _WIN32
        WSACleanup();
        #endif
        exit(-1);
    }

    sockaddr_in addr;

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr = *((const in_addr*)he->h_addr);
    memset(&(addr.sin_zero), '\0', 8);

    if (::connect(_sockfd, (sockaddr*)&addr, sizeof(addr)) == -1) {
        log->warn("Could not connect to hostname: " + hostname);
        #ifdef _WIN32
        closesocket(_sockfd);
        #else
        close(_sockfd);
        #endif
        exit(-1);
    }

    _connected = true;
    return _connected;
}

void IRCSocket::disconnect() {
    if (_connected) {
        #ifdef _WIN32
        closesocket(_sockfd);
        #else
        close(_sockfd);
        #endif
        _connected = false;
    }
}

std::string IRCSocket::sread() {
    char buffer[MAX_MESSAGE];

    memset(buffer, 0, MAX_MESSAGE);

    struct timeval r_timeout;
    r_timeout.tv_sec = 2;
    r_timeout.tv_usec = 0;
    
    fd_set read_fd;
    FD_ZERO(&read_fd);
    FD_SET(_sockfd, &read_fd);

    int rv = select(_sockfd+1, &read_fd, nullptr, nullptr, &r_timeout);
    if (rv > 0) {
        int res = recv(_sockfd, buffer, MAX_MESSAGE - 2, 0);
        if (res > 0) {
            std::string read_s = std::string(buffer);
            read_s.pop_back();
            return read_s;
        } else {
            log->warn("Bad sread(): disconnecting");
            disconnect();
        }
    } else if (rv == 0) {
        // timeout occured
    } else {
        // error occured
        log->warn("Select returned: " + std::to_string(rv));
        disconnect();
        return "";
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
