/*
 * Copyright (c) 2015 Devin Carr
 * Licensed under MIT (https://github.com/devincarr/echo/blob/master/LICENSE)
 */

#pragma once

#include "echo/logger.h"

#include <cassert>
#include <cstring>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <errno.h>

class IRCSocket {
private:
  int _sockfd;
  bool _connected;
  Log* log;

public:
  IRCSocket(Log* l);
  ~IRCSocket();

  bool connect(std::string hostname, int port);
  void disconnect();
  std::string sread();
  bool swrite(std::string msg);

  bool connected() { return _connected; }
};
