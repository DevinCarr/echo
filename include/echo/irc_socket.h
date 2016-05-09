/*
 * Copyright (c) 2015 Devin Carr
 * Licensed under MIT (https://github.com/devincarr/echo/blob/master/LICENSE)
 */

#pragma once

#include <memory>
#include <iostream>
#include <string>

#ifdef _WIN32
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define NOMINMAX
#include <WS2tcpip.h>
#include <Winsock2.h>
#include <Windows.h>
#pragma comment(lib, "Ws2_32.lib")
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <cassert>
#include <cstring>
#endif

#include "spdlog/spdlog.h"

class IRCSocket {
private:
  int _sockfd;
  bool _connected;
  std::shared_ptr<spdlog::logger> log;

public:
  IRCSocket();
  ~IRCSocket();

  bool connect(std::string hostname, int port);
  void disconnect();
  std::string sread();
  bool swrite(std::string msg);

  bool connected() { return _connected; }
};
