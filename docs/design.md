### Deadlock Strategy Choice

1. Which strategy did you choose (prevention or detection)?
We chose Deadlock Prevention using Lock Ordering (Strategy A).

2. Why did you choose this strategy?
This is a proactive and lightweight solution, which is why we selected it. Prevention guarantees that a deadlock never occurs in the first place, in contrast to Deadlock Detection (Strategy B), which necessitates a complicated background thread to execute Depth-First Search (DFS) to discover cycles. It removes the possibility of livelocks, which frequently arise with "Back-off and Retry" techniques, and saves the expense of rolling back transactions.

3. If prevention: Prove that lock ordering eliminates circular wait. Which Coffman condition is broken?
The Circular Wait condition is specifically broken by this tactic. We put a rigorous partial ordering on resource acquisition by enforcing a rule where the account with the lowest ID is always locked first (e.g., if a transfer involves Account 10 and Account 20, the thread must acquire Lock 10 then Lock 20). It is mathematically impossible to create a closed chain of dependencies in which Thread A holds Lock 10 and waits for 20 while Thread B holds Lock 20 and waits for 10 because all threads follow the same ascending order.

### Buffer Pool Configuration
1. When do you load accounts into the buffer pool?
We implement a Lazy-Load Strategy. Accounts are loaded into the pool immediately before an operation (Deposit, Withdraw, or Transfer) begins execution.

2. When do you unload them?
Regardless of whether the transaction has any outstanding activities, accounts are removed from the pool as soon as the particular banking operation is finished.

3. What happens if the pool is full when a transaction needs an account?
The transaction thread will block on the sem_wait(&empty_slots) function if all five slots are filled. Until another thread finishes its task and runs unload_account, which increases the semaphore and awakens the stuck thread, it will be in the "Waiting" state.

4. Justify your design with reasoning about performance and correctness
The system's throughput is maximized when accounts are unloaded right away following each operation rather than being held until the conclusion of a lengthy transaction. This avoids "buffer hogging," in which a sluggish transaction may choke other threads by blocking all five slots. Using semaphores guarantees that we never go over the pool's physical memory limit, avoiding race situations when allocating slots.


### Reader-Writer Lock Performance
1. Show benchmark results comparing pthread_mutex_t vs pthread_rwlock_t
While a normal mutex caused wait ticks to grow linearly as threads queued for the same account lock, the pthread_rwlock_t implementation consistently produced 0 wait ticks for balance inquiries in our tests using trace_readers.txt.

2. On which workload (trace file) does rwlock show the biggest improvement?
The workload that has improved the most is trace_readers.txt. There are a lot of BALANCE inquiries in this trail that target the same accounts at the same time.


3. Why does rwlock help on read-heavy workloads?
A Reader-Writer lock permits many threads to hold a "Read Lock" simultaneously, in contrast to a mutex, which is strictly exclusive. Transactions can check account balances concurrently in read-heavy situations without interfering with one another; they only pause when a "Writer" (Deposit/Withdraw) has to make changes to the data.



### Timer Thread Design
1. Why is a separate timer thread necessary?
A different timer thread serves as the "System Heartbeat." It offers a global, synchronized reference of time (global_tick) that is unaffected by how quickly individual transaction threads execute. Accurately measuring performance measures like "Wait Ticks" requires this.


2. What would break if you removed the timer and processed operations sequentially?
The concept of "concurrency" in the logs would not exist without the timer. Simply said, transactions would execute in the order that they were parsed. We would no longer be able to replicate "bottlenecks" in which several transactions compete for the same account at the same instant.


3. How does the timer thread enable true concurrency testing?
We can set up several transaction threads to "wake up" at the same start_tick by using the timer thread. The only method to cause and test for race situations, deadlocks, and synchronization overhead is for the operating system's scheduler to manage several threads hitting the bank and buffer_pool modules at once.
