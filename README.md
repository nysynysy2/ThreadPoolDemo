# ThreadPoolDemo
Thread Pool Demo

functions:

```cpp
ThreadPool(threadCount(optional)); //your PC's thread count by default.
addTask(delay(optional),yourFunction,yourArguments); //you must use std::chrono for the delay.
wait(); //wait for the currently running task to finish;
close(); //close the thread pool, join all threads.``` 