#include <stdio.h>
#include "lock_mgr.h"
#include "bank.h"
#include "timer.h"

extern Bank bank;

// returns wait_ticks, or -1 if insufficient funds.
int transfer(int from_id, int to_id, int amount_centavos)
{
    // lock smaller id first to break circular wait (strategy A)
    int first = (from_id < to_id) ? from_id : to_id;
    int second = (from_id < to_id) ? to_id : from_id;

    Account *acc_first = &bank.accounts[first];
    Account *acc_second = &bank.accounts[second];

    pthread_mutex_lock(&tick_lock);

    // OPTIONAL: print lock ordering for debugging
    printf("[DEADLOCK PREVENTION] Lock ordering applied: %d -> %d\n",
           first,
           second);

    int tick_before = global_tick;

    pthread_mutex_unlock(&tick_lock);

    pthread_rwlock_wrlock(&acc_first->lock);
    pthread_rwlock_wrlock(&acc_second->lock);

    pthread_mutex_lock(&tick_lock);
    int tick_after = global_tick;
    pthread_mutex_unlock(&tick_lock);

    Account *from_acc = &bank.accounts[from_id];

    // check funds after getting both locks
    if (from_acc->balance_centavos < amount_centavos)
    {
        // unlock in reverse order to be safe
        pthread_rwlock_unlock(&acc_second->lock);
        pthread_rwlock_unlock(&acc_first->lock);
        return -1;
    }

    // move the money
    bank.accounts[from_id].balance_centavos -= amount_centavos;
    bank.accounts[to_id].balance_centavos += amount_centavos;

    pthread_rwlock_unlock(&acc_second->lock);
    pthread_rwlock_unlock(&acc_first->lock);

    return (tick_after - tick_before);
}