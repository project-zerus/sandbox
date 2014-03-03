
#include "stdio.h"
#include "unistd.h"

#include "functional"

#include "toft/system/threading/condition_variable.h"
#include "toft/system/threading/mutex.h"
#include "toft/system/threading/thread.h"

static void func1(toft::Mutex* mutex) {
  mutex->Lock();
  printf("Thread 1 Locked\n");
  sleep(10);
  mutex->Unlock();
  printf("Thread 1 Unlocked\n");
}

static void func2(toft::Mutex* mutex) {
  mutex->Lock();
  printf("Thread 2 Locked\n");
  toft::ConditionVariable condvar(mutex);
  if (condvar.TimedWait(100)) {
    printf("Thread 2 TimedWait Good\n");
  } else {
    printf("Thread 2 TimedWait Bad\n");
    mutex->AssertLocked();
  }
  mutex->Unlock();
  printf("Thread 2 Unlocked\n");
}

int main(int argc, char** argv) {
  toft::Mutex mutex;
  toft::Thread t1;
  toft::Thread t2;
  t2.Start(std::bind(func2, &mutex));
  t1.Start(std::bind(func1, &mutex));
  t1.Join();
  t2.Join();
  return 0;
}
