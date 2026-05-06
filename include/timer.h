#ifndef TIMER_H
#define TIMER_H

#include <pthread.h>
#include <stdbool.h>

// Global clock
volatile int global_tick = 0;
pthread_mutex_t tick_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t tick_changed = PTHREAD_COND_INITIALIZER;
bool simulation_running = true;

// Functions
void *timer_thread(void *arg);
void wait_until_tick(int target_tick);

#endif