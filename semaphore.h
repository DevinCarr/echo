#ifndef _semaphore_h_
#define _semaphore_h_

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

#endif
