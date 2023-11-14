#include <Windows.h>
#include "Timer.h"

using namespace MyUtil;

Timer::Timer() :
	mdDeltaTime { -1.0 },
	mfScale {1.0f},
	mbStopped {false},
	mdSeondsPerCount {0.0},
	mllBaseTime {0},
	mllPausedTime {0},
	mllStopTime {0},
	mllPrevTime {0},
	mllCurrTime {0}
{
	long long countsPerSec;
	QueryPerformanceFrequency(reinterpret_cast<LARGE_INTEGER*>(&countsPerSec));

	mdSeondsPerCount = 1.0 / countsPerSec;

}

void Timer::Start()
{
	long long currTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&currTime); // È½¼ö

	mllBaseTime = currTime;
	mllPrevTime = currTime;
	mllStopTime = 0;
	mbStopped = false;

}

void MyUtil::Timer::Stop()
{
	if (!mbStopped)
	{
		long long currTime;
		QueryPerformanceCounter((LARGE_INTEGER*)&currTime); // È½¼ö

		mllStopTime = currTime;
		mbStopped = true;
	}
}

void MyUtil::Timer::Resume()
{
	if (mbStopped)
	{
		long long currTime;
		QueryPerformanceCounter((LARGE_INTEGER*)&currTime); // È½¼ö

		mllPausedTime += (currTime - mllStopTime);
		mllPrevTime = currTime;
		mllStopTime = 0;
		mbStopped = false;

	}

}

void MyUtil::Timer::Update()
{
}

float MyUtil::Timer::TotalTime() const
{
	return 0.0f;
}

float MyUtil::Timer::DeltaTime() const
{
	return 0.0f;
}

void MyUtil::Timer::SetScale(float scale)
{
}
