#Deadlock detector 

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

