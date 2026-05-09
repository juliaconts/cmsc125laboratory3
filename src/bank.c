#include <stdio.h>
#include "bank.h"
#include "timer.h" // need this to check global_tick for wait times

// global bank instance
Bank bank; 

int get_balance(int account_id)
{
    Account *acc = &bank.accounts[account_id];

    // readers can go at the same time, so just a read lock here
    pthread_rwlock_rdlock(&acc->lock);
    int balance = acc->balance_centavos;
    pthread_rwlock_unlock(&acc->lock);

    return balance;
}

int deposit(int account_id, int amount_centavos)
{
    Account *acc = &bank.accounts[account_id];

    int tick_before = global_tick; // start stopwatch
    pthread_rwlock_wrlock(&acc->lock); // need exclusive write access
    int tick_after = global_tick;  // stop stopwatch

    acc->balance_centavos += amount_centavos;

    pthread_rwlock_unlock(&acc->lock);

    return (tick_after - tick_before);
}

int withdraw(int account_id, int amount_centavos)
{
    Account *acc = &bank.accounts[account_id];

    int tick_before = global_tick;
    pthread_rwlock_wrlock(&acc->lock);
    int tick_after = global_tick;

    // check if they're broke before taking money
    if (acc->balance_centavos < amount_centavos)
    {
        pthread_rwlock_unlock(&acc->lock);
        return -1; // -1 means abort
    }

    acc->balance_centavos -= amount_centavos;
    pthread_rwlock_unlock(&acc->lock);

    return (tick_after - tick_before);
}

// sums up everything in the vault to prove we didn't lose any money
int get_total_bank_balance() 
{
    int total = 0;
    
    // lock the whole bank down so nobody moves money while we audit
    pthread_mutex_lock(&bank.bank_lock);
    for (int i = 0; i < bank.num_accounts; i++) 
    {
        pthread_rwlock_rdlock(&bank.accounts[i].lock);
        total += bank.accounts[i].balance_centavos;
        pthread_rwlock_unlock(&bank.accounts[i].lock);
    }
    pthread_mutex_unlock(&bank.bank_lock);
    
    return total;
}