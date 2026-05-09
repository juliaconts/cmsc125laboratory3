#include <stdio.h>
#include "metrics.h"
#include "bank.h"
#include "timer.h"

void run_conservation_check()
{
    printf("\n--- Conservation Check ---\n");
    int total = get_total_bank_balance();
    printf("Current Total Bank Balance: %d centavos\n", total);
}

void print_metrics(Transaction *txs, int count)
{
    int committed = 0;
    int aborted = 0;

    int total_wait = 0;

    printf("\n=== Transaction Performance Metrics ===\n");

    printf("TxID | StartTick | ActualStart | End | WaitTicks | Status\n");
    printf("-----|-----------|-------------|-----|-----------|----------\n");

    for (int i = 0; i < count; i++)
    {
        Transaction *tx = &txs[i];

        total_wait += tx->wait_ticks;

        if (tx->status == TX_COMMITTED)
            committed++;
        else
            aborted++;

        printf("T%-3d | %9d | %11d | %3d | %9d | %s\n",
               tx->tx_id,
               tx->start_tick,
               tx->actual_start,
               tx->actual_end,
               tx->wait_ticks,
               (tx->status == TX_COMMITTED)
                   ? "COMMITTED"
                   : "ABORTED");
    }

    float avg_wait = 0.0f;

    if (count > 0)
        avg_wait = (float)total_wait / count;

    int total_ticks = global_tick + 1;

    float throughput = 0.0f;

    if (total_ticks > 0)
        throughput = (float)count / total_ticks;

    printf("\nAverage wait time: %.1f ticks\n",
           avg_wait);

    printf("Throughput: %d transactions / %d ticks = %.2f tx/tick\n",
           count,
           total_ticks,
           throughput);
}