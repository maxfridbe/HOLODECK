/*
	Wiktor Kopec & Mark Tulewicz
	Last Modified 04/25/04
*/

#include "wtime.h"

LARGE_INTEGER Time::frequency = {0};
LARGE_INTEGER Time::startTime = {0};

int Time::fps = 0;
int Time::counter = 0;
double Time::accum = 0.0;
double Time::lastTime = 0.0;
double Time::interval = 0.0;
bool Time::init = false;

void Time::Start()
{
	if (!init)
	{
		QueryPerformanceFrequency(&frequency);
		init = true;
	}
	QueryPerformanceCounter(&startTime);
}

__int64 Time::getElapsedCount() 
{
	LARGE_INTEGER current;
	QueryPerformanceCounter(&current);
	return current.QuadPart - startTime.QuadPart;
}

__int64 Time::getFrequency() 
{
	return frequency.QuadPart;
}

double Time::getElapsedTime()
{
	LARGE_INTEGER current;
	QueryPerformanceCounter(&current);
	return (static_cast<double>((current.QuadPart - startTime.QuadPart)) / static_cast<double>(frequency.QuadPart));
}

double Time::getInterval()
{
	return interval;
}

int Time::getFPS()
{
	return fps;
}

void Time::updateTime()
{
	bool static first = true;
	if ( first )
	{
		lastTime = getElapsedTime();
		first = false;
		interval = 0;
		fps = 0;
	}
	else
	{
		double elapsedTime = getElapsedTime();
		interval = elapsedTime - lastTime;
		lastTime = elapsedTime;

		accum += interval;

		if ( accum >= 1.0 )
		{
			fps = counter;
			counter = 0;
			accum = 0.0;
		}
		else
		{
			counter++;
		}
	}
}
