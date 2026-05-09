### Deadlock Strategy Choice

1. Which strategy did you choose (prevention or detection)?
2. Why did you choose this strategy?
3. If prevention: Prove that lock ordering eliminates circular wait. Which Coffman condition is broken?

### Buffer Pool Configuration
1. When do you load accounts into the buffer pool?
2. When do you unload them?
3. What happens if the pool is full when a transaction needs an account?
4. Justify your design with reasoning about performance and correctness


### Reader-Writer Lock Performance
1. Show benchmark results comparing pthread_mutex_t vs pthread_rwlock_t
2. On which workload (trace file) does rwlock show the biggest improvement?
3. Why does rwlock help on read-heavy workloads?

### Timer Thread Design
1. Why is a separate timer thread necessary?
2. What would break if you removed the timer and processed operations sequentially?
3. How does the timer thread enable true concurrency testing?