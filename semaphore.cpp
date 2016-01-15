/*
 * Copyright (c) 2015 Devin Carr
 * Licensed under MIT (https://github.com/devincarr/echo/blob/master/LICENSE)
 */

#include "semaphore.h"

Semaphore::Semaphore(int _val): value(_val) {
  pthread_mutex_init(&m,NULL);
  pthread_cond_init(&c,NULL);
}

Semaphore::~Semaphore() {
  pthread_mutex_destroy(&m);
  pthread_cond_destroy(&c);
}

int Semaphore::P() {
  pthread_mutex_lock(&m);
  value--;
  if (value < 0)
    pthread_cond_wait(&c,&m);
  pthread_mutex_unlock(&m);
  return value;
}

int Semaphore::V() {
  pthread_mutex_lock(&m);
  value++;
  pthread_mutex_unlock(&m);
  if (value <= 0)
    pthread_cond_signal(&c);
  return value;
}
