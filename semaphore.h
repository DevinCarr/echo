/*
 * Copyright (c) 2015 Devin Carr
 * Licensed under MIT (https://github.com/devincarr/echo/blob/master/LICENSE)
 */

#pragma once

#include <condition_variable>
#include <mutex>

class Semaphore {
private:
  int value;
  std::mutex m;
  std::condition_variable c;

public:
  Semaphore(int _val);
  ~Semaphore();
  int P();
  int V();
};
