#ifndef METRICS_H
#define METRICS_H

#include "transaction.h"

void print_metrics(Transaction *txs, int count);
void run_conservation_check();

#endif