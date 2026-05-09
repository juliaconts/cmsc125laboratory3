#include <stdio.h>
#include "metrics.h"
#include "bank.h"

void run_conservation_check()
{
    printf("\n--- Conservation Check ---\n");
    int total = get_total_bank_balance();
    printf("Current Total Bank Balance: %d centavos\n", total);
}

void print_metrics(Transaction *txs, int count)
{
    int committed = 0, aborted = 0;
    float avg_wait = 0;

    printf("\n--- Transaction Report ---\n");
    printf("ID\tStatus\t\tStart\tEnd\tWait (Ticks)\n");
    for (int i = 0; i < count; i++)
    {
        if (txs[i].status == TX_COMMITTED)
            committed++;
        else
            aborted++;

        avg_wait += txs[i].wait_ticks;

        printf("%d\t%s\t%d\t%d\t%d\n",
               txs[i].tx_id,
               (txs[i].status == TX_COMMITTED ? "COMMITTED" : "ABORTED"),
               txs[i].actual_start, txs[i].actual_end, txs[i].wait_ticks);
    }

    printf("\nSummary:\nCommits: %d\nAborts: %d\nAvg Lock Wait: %.2f ticks\n",
           committed, aborted, avg_wait / count);
}