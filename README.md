# ThreadPoolDemo
Thread Pool Demo

functions:

```cpp
ThreadPool(threadCount(optional)); //your PC's thread count by default.
std::shared_future<(your functions return type)> addTask(delay(optional),yourFunction,yourArguments); //you must use std::chrono for the delay.
//And use shared_future::get() to get the return value of your function. 
void wait(); //wait for the currently running task to finish;
void close(); //close the thread pool, join all threads.``` 
