#include <stdio.h>
#include <stdlib.h>
#include "transaction.h"
#include "timer.h"
#include "buffer_pool.h"
#include "lock_mgr.h"
#include "bank.h"

extern BufferPool buffer_pool;

void *transaction_runner(void *arg)
{
    Transaction *tx = (Transaction *)arg;

    // 1. Wait for start time
    wait_until_tick(tx->start_tick);
    tx->actual_start = global_tick;
    tx->wait_ticks = 0;

    // 2. Execute
    execute_transaction(tx);

    tx->actual_end = global_tick;
    return NULL;
}

void execute_transaction(Transaction *tx)
{
    for (int i = 0; i < tx->num_ops; i++)
    {
        Operation *op = &tx->ops[i];
        int res = 0;

        // Ensure account is in buffer (Simulating disk I/O)
        get_account_from_buffer(&buffer_pool, op->account_id, 1);
        if (op->type == OP_TRANSFER)
        {
            get_account_from_buffer(&buffer_pool, op->target_account, 1);
        }

        // Execute Operation
        switch (op->type)
        {
        case OP_DEPOSIT:
            res = deposit(op->account_id, op->amount_centavos);
            break;
        case OP_WITHDRAW:
            res = withdraw(op->account_id, op->amount_centavos);
            break;
        case OP_TRANSFER:
            res = transfer(op->account_id, op->target_account, op->amount_centavos);
            break;
        case OP_BALANCE:
            int balance = get_balance(op->account_id);

            printf("T%d: Account %d balance = PHP %d.%02d\n",
                   tx->tx_id,
                   op->account_id,
                   balance / 100,
                   balance % 100);

            res = 0;
            break;
        }

        if (res == -1)
        {
            tx->status = TX_ABORTED;
            return; // Exit loop on failure (insufficient funds)
        }

        // Track how many ticks were spent waiting for locks
        tx->wait_ticks += res;
    }
    tx->status = TX_COMMITTED;

    // Cleanup: unload accounts from buffer
    for (int j = 0; j < tx->num_ops; j++)
    {
        unload_account(&buffer_pool, tx->ops[j].account_id);
        if (tx->ops[j].type == OP_TRANSFER)
            unload_account(&buffer_pool, tx->ops[j].target_account);
    }
}