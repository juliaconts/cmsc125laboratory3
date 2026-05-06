#include "buffer_pool.h"
#include "bank.h"
#include <string.h>
#include <stdio.h>

// Initialize buffer pool
void init_buffer_pool(BufferPool *pool)
{
    sem_init(&pool->empty_slots, 0, BUFFER_POOL_SIZE);
    sem_init(&pool->full_slots, 0, 0);
    pthread_mutex_init(&pool->pool_lock, NULL);

    for (int i = 0; i < BUFFER_POOL_SIZE; i++)
    {
        pool->slots[i].in_use = false;
        pool->slots[i].account_id = -1;
        pool->slots[i].data = NULL;
    }
}

// Load account into buffer (producer)
void load_account(BufferPool *pool, int account_id)
{
    sem_wait(&pool->empty_slots); // Wait for empty slot

    pthread_mutex_lock(&pool->pool_lock);

    // Find empty slot and load account
    for (int i = 0; i < BUFFER_POOL_SIZE; i++)
    {
        if (!pool->slots[i].in_use)
        {
            pool->slots[i].account_id = account_id;
            pool->slots[i].data = &bank.accounts[account_id];
            pool->slots[i].in_use = true;
            break;
        }
    }

    pthread_mutex_unlock(&pool->pool_lock);

    sem_post(&pool->full_slots); // Signal slot is full
}

// Unload account from buffer pool (consumer)
void unload_account(BufferPool *pool, int account_id)
{
    sem_wait(&pool->full_slots); // Wait for full slot

    pthread_mutex_lock(&pool->pool_lock);

    // Find and unload account
    for (int i = 0; i < BUFFER_POOL_SIZE; i++)
    {
        if (pool->slots[i].in_use &&
            pool->slots[i].account_id == account_id)
        {
            pool->slots[i].in_use = false;
            pool->slots[i].account_id = -1;
            break;
        }
    }

    pthread_mutex_unlock(&pool->pool_lock);

    sem_post(&pool->empty_slots); // Signal slot is empty
}

// Destroy buffer pool
void destroy_buffer_pool(BufferPool *pool)
{
    sem_destroy(&pool->empty_slots);
    sem_destroy(&pool->full_slots);
    pthread_mutex_destroy(&pool->pool_lock);
}

// Helper function (lazy-load strategy)
Account *get_account_from_buffer(BufferPool *pool, int account_id, int auto_load)
{
    Account *result = NULL;

    pthread_mutex_lock(&pool->pool_lock);

    // 1. Check if already in buffer
    for (int i = 0; i < BUFFER_POOL_SIZE; i++)
    {
        if (pool->slots[i].in_use &&
            pool->slots[i].account_id == account_id)
        {
            result = pool->slots[i].data;
            break;
        }
    }

    pthread_mutex_unlock(&pool->pool_lock);

    // 2. Lazy load if not found
    if (!result && auto_load)
    {
        load_account(pool, account_id);
        result = &bank.accounts[account_id];
    }

    return result;
}
