#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>
#include "timer.h"

// Global simulation clock (shared by all threads)
volatile int global_tick = 0;

pthread_mutex_t tick_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t tick_changed = PTHREAD_COND_INITIALIZER;

bool simulation_running = true;

// Timer thread increments clock
void *timer_thread(void *arg)
{
    (void)arg; // silence the "unused parameter" warning

    // Fetch the tick interval parsed from the command line in main.c
    extern int tick_interval_ms; 

    while (1) 
    {
        // Sleep outside the lock so we don't freeze the whole bank
        usleep(tick_interval_ms * 1000); 

        pthread_mutex_lock(&tick_lock);

        // Check the condition safely INSIDE the lock
        if (!simulation_running)
        {
            pthread_cond_broadcast(&tick_changed);
            pthread_mutex_unlock(&tick_lock);
            break; // Exit the loop safely
        }

        global_tick++;
        pthread_cond_broadcast(&tick_changed); // Wake waiting transactions
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