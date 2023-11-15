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

	mdSeondsPerCount = 1.0 / static_cast<double>(countsPerSec);

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

void Timer::Resume()
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

void Timer::Update()
{
	if (mbStopped)
	{
		mdDeltaTime = 0.0;
		return;
	}

	long long currTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&currTime); // È½¼ö
	mllCurrTime = currTime;

	mdDeltaTime = (mllCurrTime - mllPrevTime) * mdSeondsPerCount;

	mllPrevTime = mllCurrTime;

	if (mdDeltaTime < 0.0)
	{
		mdDeltaTime = 0.0;
	}
}

float Timer::TotalTime() const
{
	if (mbStopped)
	{
		return static_cast<float>(mllStopTime - mllBaseTime - mllPausedTime) * mdSeondsPerCount;
	}
	else {
		return static_cast<float>(mllCurrTime - mllBaseTime - mllPausedTime) * mdSeondsPerCount;
	}
}

float Timer::DeltaTime() const
{
	return static_cast<float> (mdDeltaTime * mfScale);
}

void Timer::SetScale(float scale)
{
	mfScale = scale;
}
