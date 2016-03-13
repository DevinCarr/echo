/*
 * Copyright (c) 2015 Devin Carr
 * Licensed under MIT (https://github.com/devincarr/echo/blob/master/LICENSE)
 */

#include "semaphore.h"

Semaphore::Semaphore(int _val): value(_val) { }

Semaphore::~Semaphore() { }

int Semaphore::P() {
    std::unique_lock<std::mutex> l(m);
    value--;
    if (value < 0)
        c.wait(l);
    return value;
}

int Semaphore::V() {
    std::unique_lock<std::mutex> l(m);
    value++;
    if (value <= 0)
        c.notify_one();
    return value;
}
