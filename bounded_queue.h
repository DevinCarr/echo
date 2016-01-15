/*
 * Copyright (c) 2015 Devin Carr
 * Licensed under MIT (https://github.com/devincarr/echo/blob/master/LICENSE)
 */

#pragma once

#include "semaphore.h"
#include <string>
#include <queue>

template <class T>
class BoundedQueue {
  Semaphore full;
  Semaphore empty;
  Semaphore mutex;

  std::queue<T> data;

public:
  BoundedQueue(): full(0), empty(1), mutex(1) { }
  BoundedQueue(int);
  ~BoundedQueue() { }
  T pull();
  void push(T);
};

template <class T>
BoundedQueue<T>::BoundedQueue(int size): full(0), empty(size), mutex(1) { }

template <class T>
T BoundedQueue<T>::pull() {
  full.P();
  mutex.P();
  T t = data.front();
  data.pop();
  mutex.V();
  empty.V();
  return t;
}

template <class T>
void BoundedQueue<T>::push(T t) {
  empty.P();
  mutex.P();
  data.push(t);
  mutex.V();
  full.V();
}
