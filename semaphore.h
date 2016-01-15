/*
 * Copyright (c) 2015 Devin Carr
 * Licensed under MIT (https://github.com/devincarr/echo/blob/master/LICENSE)
 */

#pragma once

#include <pthread.h>

class Semaphore {
private:
  int value;
  pthread_mutex_t m;
  pthread_cond_t c;

public:
  Semaphore(int _val);
  ~Semaphore();
  int P();
  int V();
};
