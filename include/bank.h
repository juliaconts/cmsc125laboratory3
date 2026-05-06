#define MAX_ACCOUNTS 100

typedef struct
{
    int account_id;        // Account number
    int balance_centavos;  // Balance in centavos
    pthread_rwlock_t lock; // Per-account lock
} Account;

typedef struct
{
    Account account[MAX_ACCOUNTS];
    int num_accounts;          // Total number of accounts
    pthread_mutex_t bank_lock; // Global lock for account management
} Bank;
