#include "stdafx.h"
#include "ThreadPool.h"

frostwave::Thread::Thread()
{
	myWorker = std::thread(&Thread::QueueLoop, this);
}

frostwave::Thread::~Thread()
{
	if (myWorker.joinable())
	{
		Wait();
		myQueueMutex.lock();
		myDestroying = true;
		myCV.notify_one();
		myQueueMutex.unlock();
		myWorker.join();
	}
}

void frostwave::Thread::AddJob(std::function<void()> aJob)
{
	std::lock_guard<std::mutex> lock(myQueueMutex);
	myJobQueue.push(std::move(aJob));
	myCV.notify_one();
}

void frostwave::Thread::Wait()
{
	std::unique_lock<std::mutex> lock(myQueueMutex);
	myCV.wait(lock, [this] { return myJobQueue.empty(); });
}

void frostwave::Thread::QueueLoop()
{
	while (true)
	{
		std::function<void()> job;

		{ //scope for unique lock
			std::unique_lock<std::mutex> lock(myQueueMutex);
			myCV.wait(lock, [this] { return !myJobQueue.empty() || myDestroying; });
			if (myDestroying) break;
			job = myJobQueue.front();
		}

		job();

		{ // scope for lock guard
			std::lock_guard<std::mutex> lock(myQueueMutex);
			myJobQueue.pop();
			myCV.notify_one();
		}
	}
}

frostwave::ThreadPool::ThreadPool()
{
}

frostwave::ThreadPool::~ThreadPool()
{
}

void frostwave::ThreadPool::SetThreadCount(u32 aCount)
{
	myThreads.clear();
	for (i32 i = 0; i < (i32)aCount; ++i)
	{
		myThreads.push_back(std::make_unique<Thread>());
	}
}

auto& frostwave::ThreadPool::GetThreads()
{
	return myThreads;
}

void frostwave::ThreadPool::Wait()
{
	for (auto& thread : myThreads)
	{
		thread->Wait();
	}
}