#include "stdafx.h"
#include "Timer.h"

frostwave::Timer::Timer() :
	myDeltaTime(0),
	myTotalTime(0)
{
	myTime = std::chrono::high_resolution_clock::now();
	myNewTime = std::chrono::high_resolution_clock::now();
	myOldTime = std::chrono::high_resolution_clock::now();
}

void frostwave::Timer::Update()
{
	myOldTime = myNewTime;

	myNewTime = std::chrono::high_resolution_clock::now();

	myDeltaTime = std::chrono::duration_cast<std::chrono::duration<f32, std::ratio<1>>>(myNewTime - myOldTime).count();

	myTotalTime = std::chrono::duration_cast<std::chrono::duration<double, std::ratio<1>>>(myNewTime - myTime).count();
}

f32 frostwave::Timer::GetDeltaTime() const
{
	return myDeltaTime;
}

double frostwave::Timer::GetTotalTime() const
{
	return myTotalTime;
}
