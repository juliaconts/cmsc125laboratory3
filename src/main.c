#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "bank.h"
#include "transaction.h"
#include "timer.h"
#include "buffer_pool.h"

// External globals (defined in other modules)
extern Bank bank;
extern BufferPool buffer_pool;

// Thread function (from transaction.c)
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

    int id, balance;

    pthread_mutex_lock(&bank.bank_lock);

    bank.num_accounts = 0;

    while (fscanf(fp, "%d %d", &id, &balance) == 2)
    {
        bank.accounts[id].account_id = id;
        bank.accounts[id].balance_centavos = balance;
        pthread_rwlock_init(&bank.accounts[id].lock, NULL);

        bank.num_accounts++;
    }

    pthread_mutex_unlock(&bank.bank_lock);
    fclose(fp);

    return 0;
}

// Parse trace.txt
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

    char op[20];
    int id, start, acc, amt, target;

    *count = 0;

    while (fscanf(fp, "%d %d %s %d %d %d",
                  &id, &start, op, &acc, &amt, &target) >= 4)
    {
        Transaction *tx = &txs[*count];

        tx->tx_id = id;
        tx->start_tick = start;
        tx->num_ops = 1;
        tx->status = TX_RUNNING;

        Operation *o = &tx->ops[0];

        if (strcmp(op, "DEPOSIT") == 0)
            o->type = OP_DEPOSIT;
        else if (strcmp(op, "WITHDRAW") == 0)
            o->type = OP_WITHDRAW;
        else if (strcmp(op, "TRANSFER") == 0)
            o->type = OP_TRANSFER;
        else
            o->type = OP_BALANCE;

        o->account_id = acc;
        o->amount_centavos = amt;
        o->target_account = target;

        (*count)++;
    }

    fclose(fp);
    return 0;
}

// MAIN
int main(int argc, char *argv[])
{
    parse_args(argc, argv);

    // init bank
    pthread_mutex_init(&bank.bank_lock, NULL);

    // load data
    load_accounts(config.accounts_file);
    load_transactions(config.trace_file);

    // buffer init
    init_buffer_pool(&buffer_pool);

    // start timer
    pthread_t timer;
    pthread_create(&timer, NULL, timer_thread, NULL);

    printf("=== Banking System Execution Log ===\n");
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

    return 0;
}