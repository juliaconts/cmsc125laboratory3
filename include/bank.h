#ifndef BANK_H
#define BANK_H

#include <pthread.h>

#define MAX_ACCOUNTS 100

typedef struct
{
    int account_id;        // Account number
    int balance_centavos;  // Balance in centavos
    pthread_rwlock_t lock; // Per-account lock
    char padding[64];
} Account;

typedef struct
{
    Account accounts[MAX_ACCOUNTS];
    int num_accounts;          // Total number of accounts
    pthread_mutex_t bank_lock; // Global lock for account management
} Bank;

#endif