# Deadlock detector 

## ddmon.c
- It overrides "pthread_mutex_lock" and "pthread_mutex_unlock"
- When these two function execute, information about the lock are sended through FIFO to detector


## ddtect.c
- It is a deadlock detector program detects a deadlock generated in a program whose name is given to the deadlock detector via cmd argument .
- Also, it predicts Mutexes at risk of potentially deadlock.
- The detector draws a mutex lock graph and updates the graph everytime it receives lock information (redardless of lock or unlock information).
- After updating the graph, the detector search the graph and find a cycle.
- If it finds a cycle, the program prints out the line number of the deadlock genering function.(By using "backtrace()" and "addr2line()").
- Right before the detector program ends, the daedlock predict function executes.
- It finds mutexs that fortunately does not cause a deadlock but has a risk of causing it.


## detection algorithm

- If a thread has a mutex lock, it makes a node.
- If a thread has a mutex lock(lock B) holding another(lock A), it draws an edge (Node A, Node B).
- If a thread waits a mutex lock (lock B) holding lock A, it also draws an edge (Node A, Node B).
- When unlock, it erases edges related to the relesed mutex.
- Everytime it receives lock information, it updates the graph and detects the cycle.

## predict algorithm
- The draw process is same as the detection algorithm drawing.
- But for prediction, it will not erase edges even though unlock information is given.
- Before ends the program, the deadlock predict function executes.
- Considering that a single thread cycle and a gatelock can't be a deadlock, it find cycles.

### Single thread thread & gatelock
