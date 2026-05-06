#ifndef BUFFER_POOL_H
#define BUFFER_POOL_H

#include <stdbool.h>
#include <semaphore.h>
#include <pthread.h>

#define BUFFER_POOL_SIZE 5

typedef struct Account Account; // Forward declaration to avoid circular dependency

typedef struct
{
    int account_id;
    Account *data;
    bool in_use;
} BufferSlot;

typedef struct
{
    BufferSlot slots[BUFFER_POOL_SIZE];
    sem_t empty_slots;
    sem_t full_slots;
    pthread_mutex_t pool_lock;
} BufferPool;

// API
void init_buffer_pool(BufferPool *pool);
void destroy_buffer_pool(BufferPool *pool);

void load_account(BufferPool *pool, int account_id);
void unload_account(BufferPool *pool, int account_id);

#endif