#ifndef BANK_H
#define BANK_H

#include <pthread.h>
#include <stdbool.h>

#define MAX_ACCOUNTS 100

typedef struct
{
    int account_id;        
    int balance_centavos;  
    pthread_rwlock_t lock; // per-account lock to allow multiple readers
    char padding[64];      // cache line padding to prevent false sharing
} Account;

typedef struct
{
    Account accounts[MAX_ACCOUNTS];
    int num_accounts;          
    pthread_mutex_t bank_lock; // protects the whole bank during setup and audit
} Bank;

// functions return wait_ticks, or -1 if insufficient funds
int get_balance(int account_id);
int deposit(int account_id, int amount_centavos);
int withdraw(int account_id, int amount_centavos);

// safety check for test 3
int get_total_bank_balance();

#endif