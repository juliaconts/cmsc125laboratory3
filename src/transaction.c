#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "transaction.h"
#include "timer.h"
#include "buffer_pool.h"
#include "lock_mgr.h"
#include "bank.h"

extern BufferPool buffer_pool;

void *transaction_runner(void *arg)
{
    Transaction *tx = (Transaction *)arg;

    // Wait until scheduled start
    wait_until_tick(tx->start_tick);

    pthread_mutex_lock(&tick_lock);
    tx->actual_start = global_tick;
    pthread_mutex_unlock(&tick_lock);

    tx->wait_ticks = 0;

    Operation *op = &tx->ops[0];

    pthread_mutex_lock(&tick_lock);
    printf("Tick %d:\n", global_tick);
    pthread_mutex_unlock(&tick_lock);

    switch (op->type)
    {
    case OP_DEPOSIT:
        printf("  T%d started: DEPOSIT account %d amount PHP %d.%02d\n",
               tx->tx_id,
               op->account_id,
               op->amount_centavos / 100,
               op->amount_centavos % 100);
        break;

    case OP_WITHDRAW:
        printf("  T%d started: WITHDRAW account %d amount PHP %d.%02d\n",
               tx->tx_id,
               op->account_id,
               op->amount_centavos / 100,
               op->amount_centavos % 100);
        break;

    case OP_TRANSFER:
        printf("  T%d started: TRANSFER from %d to %d amount PHP %d.%02d\n",
               tx->tx_id,
               op->account_id,
               op->target_account,
               op->amount_centavos / 100,
               op->amount_centavos % 100);
        break;

    case OP_BALANCE:
        printf("  T%d started: BALANCE account %d\n",
               tx->tx_id,
               op->account_id);
        break;
    }

    execute_transaction(tx);

    pthread_mutex_lock(&tick_lock);
    tx->actual_end = global_tick;
    pthread_mutex_unlock(&tick_lock);

    return NULL;
}

void execute_transaction(Transaction *tx)
{
    for (int i = 0; i < tx->num_ops; i++)
    {
        Operation *op = &tx->ops[i];

        int res = 0;

        // Load into buffer pool
        get_account_from_buffer(&buffer_pool,
                                op->account_id,
                                1);

        if (op->type == OP_TRANSFER)
        {
            get_account_from_buffer(&buffer_pool,
                                    op->target_account,
                                    1);
        }

        switch (op->type)
        {
        case OP_DEPOSIT:
            res = deposit(op->account_id,
                          op->amount_centavos);
            break;

        case OP_WITHDRAW:
            res = withdraw(op->account_id,
                           op->amount_centavos);
            break;

        case OP_TRANSFER:
            res = transfer(op->account_id,
                           op->target_account,
                           op->amount_centavos);
            break;

        case OP_BALANCE:
        {
            int balance = get_balance(op->account_id);

            printf("T%d: Account %d balance = PHP %d.%02d\n",
                   tx->tx_id,
                   op->account_id,
                   balance / 100,
                   balance % 100);

            res = 0;
            break;
        }
        }

        // Simulate operation time
        usleep(120000);

        if (res == -1)
        {
            tx->status = TX_ABORTED;

            // Cleanup loaded accounts
            for (int j = 0; j < tx->num_ops; j++)
            {
                unload_account(&buffer_pool,
                               tx->ops[j].account_id);

                if (tx->ops[j].type == OP_TRANSFER)
                {
                    unload_account(&buffer_pool,
                                   tx->ops[j].target_account);
                }
            }

            return;
        }

        tx->wait_ticks += res;
    }

    tx->status = TX_COMMITTED;

    pthread_mutex_lock(&tick_lock);
    printf("Tick %d:\n", global_tick);
    pthread_mutex_unlock(&tick_lock);

    printf("  T%d completed successfully\n",
           tx->tx_id);

    // Cleanup buffer pool
    for (int j = 0; j < tx->num_ops; j++)
    {
        unload_account(&buffer_pool,
                       tx->ops[j].account_id);

        if (tx->ops[j].type == OP_TRANSFER)
        {
            unload_account(&buffer_pool,
                           tx->ops[j].target_account);
        }
    }
}