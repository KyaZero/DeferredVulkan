#pragma once
#include <chrono>
namespace frostwave
{
	class Timer
	{

	public:
		Timer();
		Timer(const Timer &aTimer) = delete;
		Timer& operator=(const Timer &aTimer) = delete;

		void Update();

		f32 GetDeltaTime() const;
		double GetTotalTime() const;

	private:
		f32 myDeltaTime;
		double myTotalTime;
		std::chrono::high_resolution_clock::time_point myTime;
		std::chrono::high_resolution_clock::time_point myNewTime;
		std::chrono::high_resolution_clock::time_point myOldTime;
	};
}
namespace fw = frostwave;
