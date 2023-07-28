#ifndef _THREAD_POOL_
#define _THREAD_POOL_
#include <thread>
#include <future>
#include <iostream>
#include <type_traits>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <list>
#include <atomic>
class ThreadPool
{
public:
	std::atomic<bool> isExit = false;
	std::atomic<bool> isTerminate = false;
	std::atomic<int> wokingThreadCount = 0;
	std::vector<std::thread> thread_pool;
	std::list<std::packaged_task<void()>> cache;
	std::mutex m_lock;
	std::condition_variable cv;
	std::condition_variable cv2;
	explicit ThreadPool(size_t thread_count = std::thread::hardware_concurrency());
	explicit ThreadPool(const ThreadPool&) = delete;
	explicit ThreadPool(ThreadPool&&) = delete;
	void _execThread();
	void wait();
	template<class Fn,class... Args>
	std::shared_future<typename std::invoke_result<Fn, Args...>::type> addTask(Fn&& func, Args&&... args);
	template<class Fn, class... Args, class Dura>
	std::shared_future<typename std::invoke_result<Fn, Args...>::type> addTask(Dura&& dura, Fn&& func, Args&&... args);
	void close();
	auto& operator[](size_t index) { return thread_pool[index]; }
	~ThreadPool() {
		this->isExit = true;
		if (!(this->isTerminate.load())) {
			for (int i = 0; i < this->thread_pool.size(); ++i)
				this->addTask([] {});
			for (auto& e : this->thread_pool)
				e.join();
		}
	}
};
ThreadPool::ThreadPool(size_t thread_count) {
	for (size_t i = 0; i < thread_count; ++i)
		thread_pool.push_back(std::thread(&ThreadPool::_execThread, this));
}
void ThreadPool::_execThread()
{
	while (!(isExit.load()))
	{
		std::unique_lock<std::mutex> locker(m_lock);
		cv.wait(locker, [=] {return !(this->cache.empty()); });
		std::packaged_task<void()> task = std::move(this->cache.front());
		this->cache.pop_front();
		locker.unlock();
		++wokingThreadCount;
		task();
		--wokingThreadCount;
		cv2.notify_all();
	}
}
template<class Fn, class... Args>
std::shared_future<typename std::invoke_result<Fn, Args...>::type> ThreadPool::addTask(Fn&& func, Args&&... args)
{
	auto m_future = std::async(std::launch::deferred, std::forward<Fn>(func), std::forward<Args>(args)...);
	auto s_f = m_future.share();
	std::packaged_task<void()> task([s_f]() mutable {s_f.wait(); });
	std::lock_guard<std::mutex> locker(m_lock);
	cache.push_back(std::move(task));
	cv.notify_one();
	return s_f;
}
template<class Fn, class... Args, class Dura>
std::shared_future<typename std::invoke_result<Fn, Args...>::type> ThreadPool::addTask(Dura&& dura, Fn&& func, Args&&... args)
{
	return addTask([&]() {
		std::this_thread::sleep_for(dura);
		return std::forward<Fn>(func)(std::forward<Args>(args)...);
	});
}
void ThreadPool::close()
{
	this->wait();
	this->isExit = true;
	this->isTerminate = true;
	for (int i = 0; i < this->thread_pool.size(); ++i)
		this->addTask([]{});
	for (auto& e : this->thread_pool) {
		e.join();
	}
}
void ThreadPool::wait()
{
	if (this->isExit || this->isTerminate)return;
	std::unique_lock<std::mutex> locker(m_lock);
	cv2.wait(locker, [=]() {return (this->cache.empty()) && (this->wokingThreadCount.load() == 0); });
}
#endif
