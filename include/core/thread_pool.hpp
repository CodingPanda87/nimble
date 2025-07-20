// *************************************************************************
// [Author]		xiong.qiang
// [Date]		2024-05-02
// [Describe]	ThreadPool
// [Copyright]  xiong.qiang
// [Brief]      thread pool and administer
// *************************************************************************
#pragma once

#include <vector>
#include <queue>
#include <atomic>
#include <future>
#include <stdexcept>
#include <shared_mutex>
#include "x/x.hpp"

namespace nb
{

class ThreadPool
{    
protected:
	using Task = std::function<void()>;

    unsigned short              _initSize;
	std::mutex                  _lockPool;     
	std::vector<std::thread>    _pool;          
	std::queue<Task>            _tasks;            
	std::mutex                  _lockTask;

	std::mutex 					_lockGrow;
	std::condition_variable     _task_cv;   
	std::atomic<bool>           _run{ true };
	std::atomic<int>            _idlThreadNum{ 0 };

	std::shared_mutex           _mtxActiveFlags;
	std::unordered_map<std::thread::id, 
	          		   std::chrono::steady_clock::time_point> _activeFlag;

public:
	ThreadPool(unsigned short size = 4):
			   _initSize(size){ addThread(size); }

	~ThreadPool(){
		_run=false;
		_task_cv.notify_all();
		for (std::thread& thread : _pool) {
			if (thread.joinable())
				thread.join();
		}
	}

	int deadThreadNum() { 
		auto cnt = 0;
		auto now = std::chrono::steady_clock::now();
		std::shared_lock<std::shared_mutex> lock{ _mtxActiveFlags };
		for (auto& [_, flag] : _activeFlag) {
			if (flag != std::chrono::steady_clock::time_point()){
				auto d = std::chrono::duration_cast<std::chrono::seconds>(now - flag).count();
#ifndef MAXTIME_THREAD_IN_DEAD
				if (d >= 600)
#else
				if (d >= MAXTIME_THREAD_IN_DEAD)
#endif
					cnt++;
			} 
		}
		return cnt;
	}

	template<class F, class... Args>
	auto commit(F&& f, Args&&... args) -> std::future<decltype(f(args...))>
	{
		using RetType = decltype(f(args...)); 
		auto task = std::make_shared<std::packaged_task<RetType()>>(
			std::bind(std::forward<F>(f), std::forward<Args>(args)...)
		);
		std::future<RetType> future = task->get_future();
		if (!_run)  return future;
		{   
			std::lock_guard<std::mutex> lock{ _lockTask };
			_tasks.emplace([task]() {
				(*task)();
			});
		}
		_task_cv.notify_one();

		return future;
	}
	
	template <class F>
	void commitNoRet(F&& task)
	{
		if (!_run) return;
        {
			std::lock_guard<std::mutex> lock{ _lockTask };
			_tasks.emplace(std::forward<F>(task));
		}
		_task_cv.notify_one();
	}

	int idlSize()  { return _idlThreadNum; }

	int poolSize() { return static_cast<int>(_pool.size()); }

	void destroy(){
		_run=false;
		_task_cv.notify_all();
		for (std::thread& thread : _pool) {
			if (thread.joinable())
				thread.join();
		}
		_pool.clear();
		_tasks = std::queue<Task>();
	}

protected:

	void addThread(unsigned short size)
	{
		for (; size > 0; --size)
		{   
			_pool.emplace_back( [this]{ 
				while (_run) 
				{
					Task task; 
					{
						{	// update active flag,0 - sleeping
							std::shared_lock<std::shared_mutex> lock{ _mtxActiveFlags };
							_activeFlag[std::this_thread::get_id()] = {};
						}
						std::unique_lock<std::mutex> lock{ _lockTask };
						_task_cv.wait(lock, [this] { 
							return !_run || !_tasks.empty();
						});
						if (!_run && _tasks.empty())
							return;
						_idlThreadNum--;
						task = move(_tasks.front());
						_tasks.pop();
					}
					{	// update active flag
						std::shared_lock<std::shared_mutex> lock{ _mtxActiveFlags };
						_activeFlag[std::this_thread::get_id()] = std::chrono::steady_clock::now();
					}
					task();
					_idlThreadNum++;
				}
			});
			_idlThreadNum++;
		}
	}
};

class ThreadPoolAdmin
{
public:
	~ThreadPoolAdmin(){}

	static ThreadPoolAdmin* instance(){
		static ThreadPoolAdmin instance;
		return &instance;
	}

	ThreadPool* creat(const int &sz = 2){
		if(size() + sz > _maxSize)
			return nullptr;
		auto pool = std::make_shared<ThreadPool>(2);
		if(pool == nullptr)
			return nullptr;
		std::unique_lock<std::shared_mutex> lock{ _lock };
		_pools.push_back(pool);
		return pool.get();
	}

	size_t size() const noexcept{
		size_t sz = 0;
		std::shared_lock<std::shared_mutex> lock{ _lock };
		for (auto& pool : _pools)
			sz += pool->poolSize();
		return sz;
	}

	size_t maxSize() const noexcept{ return _maxSize; }

	void clear(){
		std::unique_lock<std::shared_mutex> lock{ _lock };
		for (auto& pool : _pools)
			pool->destroy();
		_pools.clear();
	}

	size_t deadThreadNum() const{
		size_t sz = 0;
		std::shared_lock<std::shared_mutex> lock{ _lock };
		for (auto& pool : _pools)
			sz += pool->deadThreadNum();
		return sz;
	}

protected:
	ThreadPoolAdmin():
	_maxSize(std::thread::hardware_concurrency()*2) {}

	int 										    _maxSize = 16;
	mutable std::shared_mutex 						_lock;
	std::vector<std::shared_ptr<ThreadPool>> 		_pools;
};

} //! namespace NB
