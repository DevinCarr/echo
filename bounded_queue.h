/*
 * Copyright (c) 2015 Devin Carr
 * Licensed under MIT (https://github.com/devincarr/echo/blob/master/LICENSE)
 */

#pragma once

#include "semaphore.h"
#include <mutex>
#include <queue>

template <class T>
class BoundedQueue {
    Semaphore full;
    Semaphore empty;
    std::mutex mut;

    std::queue<T> data;

public:
    BoundedQueue(): full(0), empty(1) { }
    BoundedQueue(int);
    ~BoundedQueue() { }
    T pull();
    void push(T);
};

template <class T>
BoundedQueue<T>::BoundedQueue(int size): full(0), empty(size) { }

template <class T>
T BoundedQueue<T>::pull() {
    full.P();
    mut.lock();
    T t = data.front();
    data.pop();
    mut.unlock();
    empty.V();
    return t;
}

template <class T>
void BoundedQueue<T>::push(T t) {
    empty.P();
    mut.lock();
    data.push(t);
    mut.unlock();
    full.V();
}
