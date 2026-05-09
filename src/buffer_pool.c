#include "buffer_pool.h"
#include <string.h>
#include <stdio.h>

extern Bank bank;

void init_buffer_pool(BufferPool *pool)
{
    // 5 empty chairs, 0 taken initially
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

// pull account from disk into the pool (producer)
void load_account(BufferPool *pool, int account_id)
{
    // wait for an empty chair. blocks here if pool is full.
    sem_wait(&pool->empty_slots); 

    // lock the array so threads don't fight over the same slot
    pthread_mutex_lock(&pool->pool_lock);

    // find empty slot and take it
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

    // signal that a slot is now full
    sem_post(&pool->full_slots); 
}

// kick account out of the pool (consumer)
void unload_account(BufferPool *pool, int account_id)
{
    // wait for a full slot to empty out
    sem_wait(&pool->full_slots); 

    pthread_mutex_lock(&pool->pool_lock);

    // find it and free it
    for (int i = 0; i < BUFFER_POOL_SIZE; i++)
    {
        if (pool->slots[i].in_use && pool->slots[i].account_id == account_id)
        {
            pool->slots[i].in_use = false;
            pool->slots[i].account_id = -1;
            break; 
        }
    }

    pthread_mutex_unlock(&pool->pool_lock);

    // signal that we freed up a chair
    sem_post(&pool->empty_slots); 
}

void destroy_buffer_pool(BufferPool *pool)
{
    sem_destroy(&pool->empty_slots);
    sem_destroy(&pool->full_slots);
    pthread_mutex_destroy(&pool->pool_lock);
}

// smart helper: loads it only if it isn't already chilling in the pool
Account *get_account_from_buffer(BufferPool *pool, int account_id, int auto_load)
{
    Account *result = NULL;

    pthread_mutex_lock(&pool->pool_lock);

    // check if it's already there
    for (int i = 0; i < BUFFER_POOL_SIZE; i++)
    {
        if (pool->slots[i].in_use && pool->slots[i].account_id == account_id)
        {
            result = pool->slots[i].data;
            break;
        }
    }

    pthread_mutex_unlock(&pool->pool_lock);

    // lazy load if not found
    if (!result && auto_load)
    {
        load_account(pool, account_id);
        result = &bank.accounts[account_id];
    }

    return result;
}