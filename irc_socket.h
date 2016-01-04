/*
 * Copyright (c) 2015 Devin Carr
 * Licensed under MIT (https://github.com/devincarr/echo/blob/master/LICENSE)
 */

#ifndef _irc_socket_h_
#define _irc_socket_h_ 

#include "logger.h"

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
  struct addrinfo hints;
  struct addrinfo *result;
  int _sockfd;
  bool _connected;
  Log* log;

public:
  IRCSocket(Log* l);
  ~IRCSocket();

  bool connect(char * hostname, int port);
  void disconnect();
  std::string sread();
  bool swrite(std::string msg);

  bool connected() { return _connected; }
};

#endif
