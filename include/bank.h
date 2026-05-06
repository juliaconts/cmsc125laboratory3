#ifndef BANK_H
#define BANK_H

#include <pthread.h>
#include <stdbool.h>

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

// Function prototypes
int get_balance(int account_id);
bool deposit(int account_id, int amount_centavos);
bool withdraw(int account_id, int amount_centavos);
bool transfer(int from_account, int to_account, int amount_centavos);

#endif