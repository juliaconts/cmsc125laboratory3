#include <stdio.h>
#include "bank.h"

// Global bank instance (define it ONCE here or in main.c)
extern Bank bank;

int get_balance(int account_id)
{
    Account *acc = &bank.accounts[account_id];

    pthread_rwlock_rdlock(&acc->lock);
    int balance = acc->balance_centavos;
    pthread_rwlock_unlock(&acc->lock);

    return balance;
}

void deposit(int account_id, int amount_centavos)
{
    Account *acc = &bank.accounts[account_id];

    pthread_rwlock_wrlock(&acc->lock);
    acc->balance_centavos += amount_centavos;
    pthread_rwlock_unlock(&acc->lock);
}

bool withdraw(int account_id, int amount_centavos)
{
    Account *acc = &bank.accounts[account_id];

    pthread_rwlock_wrlock(&acc->lock);

    if (acc->balance_centavos < amount_centavos)
    {
        pthread_rwlock_unlock(&acc->lock);
        return false;
    }

    acc->balance_centavos -= amount_centavos;
    pthread_rwlock_unlock(&acc->lock);
    return true;
}

bool transfer(int from_id, int to_id, int amount_centavos)
{
    // Acquire locks in consistent order to prevent deadlock
    int first = (from_id < to_id) ? from_id : to_id;
    int second = (from_id < to_id) ? to_id : from_id;

    Account *acc_first = &bank.accounts[first];
    Account *acc_second = &bank.accounts[second];

    pthread_rwlock_wrlock(&acc_first->lock);
    pthread_rwlock_wrlock(&acc_second->lock);

    // Check sufficient funds
    Account *from_acc = &bank.accounts[from_id];
    if (from_acc->balance_centavos < amount_centavos)
    {
        pthread_rwlock_unlock(&acc_second->lock);
        pthread_rwlock_unlock(&acc_first->lock);
        return false;
    }

    // Perform transfer
    bank.accounts[from_id].balance_centavos -= amount_centavos;
    bank.accounts[to_id].balance_centavos += amount_centavos;

    pthread_rwlock_unlock(&acc_second->lock);
    pthread_rwlock_unlock(&acc_first->lock);
    return true;
}
