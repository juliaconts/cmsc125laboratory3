#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include "timer.h"

// Global simulation clock (shared by all threads)
volatile int global_tick = 0;
pthread_mutex_t tick_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t tick_changed = PTHREAD_COND_INITIALIZER;
bool simulation_running = true;
int tick_interval_ms = 100; // 100 ms per tick

// Timer thread increments clock every TICK_INTERVAL_MS
void *timer_thread(void *arg)
{
    (void)arg;
    while (simulation_running)
    {
        usleep(tick_interval_ms * 1000);

        pthread_mutex_lock(&tick_lock);

        global_tick++;

        pthread_cond_broadcast(&tick_changed);

        pthread_mutex_unlock(&tick_lock);
    }

    return NULL;
}

// Transactions wait until their start_tick
void wait_until_tick(int target_tick)
{
    pthread_mutex_lock(&tick_lock);
    while (global_tick < target_tick && simulation_running)
    {
        pthread_cond_wait(&tick_changed, &tick_lock);
    }
    pthread_mutex_unlock(&tick_lock);
}