# CMSC 125 Lab 3: Concurrent Banking

## Group Members
- **Junel O. Arellano** (Backend, Buffer Management, Deadlock Prevention)
- **Julia Louise M. Contreras** (Simulation Engine, File Parsing, Time Simulation)

## Description
This project is a C simulator of a high-concurrency banking system. It simulates a financial setting in which several transactions vie for shared account access. Using a bounded buffer pool, the system replicates disk-to-memory limits and tackles traditional concurrency issues like deadlocks and race situations. To synchronize operations and offer comprehensive performance measurements, it makes use of a global simulation clock.

## Compilation and usage instructions

1. **Compilation**

    * Compile all source files using GCC:
    ```bash
    gcc src/*.c -Iinclude -pthread -o bankdb
    ```

    * With warnings and ThreadSanitizer enabled (Debug mode):
    ```bash
    gcc -Wall -Wextra -g -fsanitize=thread src/*.c -Iinclude -pthread -o bankdb
    ```

2. **Usage**

    * General command format:
    ```bash
    ./bankdb --accounts=<ACCOUNTS_FILE> --trace=<TRACE_FILE> --deadlock=<MODE>
    ```

    * Supported deadlock modes:
        a. **prevention** (Lock Ordering)
        b. **none** (No deadlock handling)

    * Additional Flags:
        - `--tick-ms=[number]`: Sets the speed of the simulation clock (default: 100ms).
        - `--verbose`: Enables detailed per-tick operation logs.

## List of implemented features
- **Thread-Safe Operations:** Atomic deposit, withdrawal, and transfer operations using `pthread_rwlock_t`.
- **Deadlock Prevention:** Implementation of **Lock Ordering** to eliminate circular wait conditions.
- **Bounded Buffer Pool:** A 5-slot memory management system using semaphores to regulate concurrent account access.
- **Lazy Loading:** Accounts are pulled from a simulated disk into the buffer only when an operation is active.
- **Global Timer Thread:** A synchronized heartbeat for the system to manage transaction scheduling and metrics.
- **Conservation Check:** An automated audit at the end of execution to verify that no money was lost or gained.
- **Performance Metrics:** Detailed tracking of start/end ticks, actual execution time, and wait-time statistics per transaction.

## Known limitations (if any)
- **Hardcoded Buffer Size:** The buffer pool is currently fixed at 5 slots as per lab requirements.
- **Account Limit:** The system supports up to 100 unique accounts.
- **Transaction Limit:** The current trace parser is limited to 256 concurrent transactions.
- **WSL2 ThreadSanitizer Bug:** If running in WSL2, you may need to run `sudo sysctl vm.mmap_rnd_bits=28` to avoid memory mapping errors with ThreadSanitizer.