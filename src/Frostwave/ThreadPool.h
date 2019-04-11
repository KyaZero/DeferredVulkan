#pragma once

#include <thread>
#include <queue>
#include <functional>
#include <mutex>

namespace frostwave
{
	class Thread
	{
	public:
		Thread();
		~Thread();

		void AddJob(std::function<void()> aJob);
		void Wait();

	private:
		void QueueLoop();

		std::thread myWorker;
		std::queue<std::function<void()>> myJobQueue;
		std::mutex myQueueMutex;
		std::condition_variable myCV;
		bool myDestroying = false;
	};

	class ThreadPool
	{
	public:
		ThreadPool();
		~ThreadPool();

		void SetThreadCount(u32 aCount);
		auto& GetThreads();
		void Wait();

	private:
		std::vector<std::unique_ptr<Thread>> myThreads;
	};
}
