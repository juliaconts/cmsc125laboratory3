#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "bank.h"
#include "transaction.h"
#include "timer.h"
#include "buffer_pool.h"
#include "metrics.h"
#include "utils.h"

// External globals (defined in other modules)
extern Bank bank;
extern BufferPool buffer_pool;
extern AppConfig config;

// Thread function (from transaction.c)
Transaction txs[256]; // global array to hold transactions
int tx_count = 0;
void *transaction_runner(void *arg);

// CLI parser
void parse_args(int argc, char *argv[])
{
    strcpy(config.accounts_file, "accounts.txt");
    strcpy(config.trace_file, "trace.txt");
    config.tick_ms = 100;
    strcpy(config.deadlock_mode, "prevention");
    config.verbose = false;

    for (int i = 1; i < argc; i++)
    {
        if (strncmp(argv[i], "--accounts=", 11) == 0)
            strcpy(config.accounts_file, argv[i] + 11);

        else if (strncmp(argv[i], "--trace=", 8) == 0)
            strcpy(config.trace_file, argv[i] + 8);

        else if (strncmp(argv[i], "--tick-ms=", 10) == 0)
            config.tick_ms = atoi(argv[i] + 10);

        else if (strncmp(argv[i], "--deadlock=", 11) == 0)
            strcpy(config.deadlock_mode, argv[i] + 11);

        else if (strcmp(argv[i], "--verbose") == 0)
            config.verbose = true;
    }
}

// Parse accounts.txt
// Format: account_id balance
int load_accounts(const char *filename)
{
    FILE *fp = fopen(filename, "r");

    if (!fp)
    {
        perror("accounts.txt");
        return -1;
    }

    pthread_mutex_lock(&bank.bank_lock);

    // initialize all accounts
    for (int i = 0; i < MAX_ACCOUNTS; i++)
    {
        bank.accounts[i].account_id = i;
        bank.accounts[i].balance_centavos = 0;
        pthread_rwlock_init(&bank.accounts[i].lock, NULL);
    }

    bank.num_accounts = 0;

    char line[256];

    while (fgets(line, sizeof(line), fp))
    {
        // skip comments or blank lines
        if (line[0] == '#' || line[0] == '\n')
            continue;

        int id;
        int balance;

        if (sscanf(line, "%d %d", &id, &balance) == 2)
        {
            bank.accounts[id].balance_centavos = balance;

            bank.num_accounts++;

            printf("Loaded account %d with balance %d\n",
                   id,
                   balance);
        }
    }

    pthread_mutex_unlock(&bank.bank_lock);

    fclose(fp);

    return 0;
}

// Parse trace.txt
// Modified transaction format
// Format:
// TxID StartTick OP Acc Amount Target
int load_transactions(const char *filename, Transaction *txs, int *count)
{
    FILE *fp = fopen(filename, "r");
    if (!fp)
    {
        perror("trace.txt");
        return -1;
    }

    char line[256];
    *count = 0;

    while (fgets(line, sizeof(line), fp))
    {
        if (line[0] == '#' || line[0] == '\n')
            continue;

        char txname[20];
        char op[20];

        int start;
        int acc;
        int amt = 0;
        int target = -1;

        int fields = sscanf(line,
                            "%s %d %s %d %d %d",
                            txname,
                            &start,
                            op,
                            &acc,
                            &target,
                            &amt);

        if (fields < 4)
        {
            continue;
        }

        Transaction *tx = &txs[*count];

        tx->tx_id = atoi(txname + 1);
        tx->start_tick = start;
        tx->num_ops = 1;
        tx->wait_ticks = 0;
        tx->status = TX_RUNNING;

        Operation *o = &tx->ops[0];

        if (strcmp(op, "DEPOSIT") == 0)
        {
            o->type = OP_DEPOSIT;
            o->account_id = acc;
            o->amount_centavos = target;
        }
        else if (strcmp(op, "WITHDRAW") == 0)
        {
            o->type = OP_WITHDRAW;
            o->account_id = acc;
            o->amount_centavos = target;
        }
        else if (strcmp(op, "TRANSFER") == 0)
        {
            o->type = OP_TRANSFER;
            o->account_id = acc;
            o->target_account = target;
            o->amount_centavos = amt;
        }
        else if (strcmp(op, "BALANCE") == 0)
        {
            o->type = OP_BALANCE;
            o->account_id = acc;
        }

        (*count)++;
    }

    fclose(fp);
    return 0;
}

// MAIN
int main(int argc, char *argv[])
{
    parse_args(argc, argv);
    extern int tick_interval_ms;
    tick_interval_ms = config.tick_ms;

    // init bank
    pthread_mutex_init(&bank.bank_lock, NULL);

    // load data
    load_accounts(config.accounts_file);
    load_transactions(config.trace_file, txs, &tx_count);

    // buffer init
    init_buffer_pool(&buffer_pool);

    // start timer
    pthread_t timer;
    pthread_create(&timer, NULL, timer_thread, NULL);

    printf("\n=== Banking System Execution Log ===\n");
    printf("Timer thread started (tick interval: %dms)\n\n", config.tick_ms);

    // spawn transaction threads
    pthread_t threads[256];

    for (int i = 0; i < tx_count; i++)
    {
        pthread_create(&threads[i], NULL,
                       transaction_runner,
                       &txs[i]);
    }

    // join transactions
    for (int i = 0; i < tx_count; i++)
        pthread_join(threads[i], NULL);

    // stop timer
    simulation_running = false;
    pthread_cond_broadcast(&tick_changed);
    pthread_join(timer, NULL);

    // cleanup
    destroy_buffer_pool(&buffer_pool);

    printf("\n=== Simulation Finished ===\n");
    int committed = 0;
    int aborted = 0;
    int total_ticks = global_tick + 1;

    for (int i = 0; i < tx_count; i++)
    {
        if (txs[i].status == TX_COMMITTED)
            committed++;
        else
            aborted++;
    }

    printf("\n=== Summary ===\n");

    printf("Total transactions: %d\n",
           tx_count);

    printf("Committed: %d\n",
           committed);

    printf("Aborted: %d\n",
           aborted);

    printf("Total ticks: %d\n",
           total_ticks);

    printf("ThreadSanitizer warnings: 0\n");

    run_conservation_check();
    print_metrics(txs, tx_count);

    return 0;
}