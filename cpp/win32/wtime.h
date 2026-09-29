/*
	Wiktor Kopec & Mark Tulewicz
	Last Modified 04/25/04
*/

#ifndef TIME_H
#define TIME_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

//<summary>Class used to keep track of time</summary>
class Time
{
	public:
		//<summary>Sets the starting point of the timer</summary>
		//<remarks>This function needs to be called at least once</summary>
		static void Start();
		
		//<summary>Gets the amount of time elapsed since the last updateTime call</summary>
		//<return>Time, in seconds</return>
		static double getInterval();
		
		//<summary>Updates the current time interval</summary>
		//<remarks>This function needs to be called on every iteration
		//of the message loop in order to keep accurate track of
		//fps</remarks>
		static void updateTime();
		
		//<summary>Gets the current frames per second</summary> 
		//<returns>The current frames per second</returns>
		static int getFPS();
		
		//<summary>Gets the processor frequency</summary>
		static __int64 getFrequency();

		//<summary>Gets the amount of cycles since Start</summary>
		static __int64 getElapsedCount();

		//<summary>Gets the amount of time since the Start call.</summary>
		//<remarks>This function is equivalent to getElapsedCount() / getFrequency()</remarks>
		static double getElapsedTime();
	private:
		static int fps;				
		static int counter;
		static double accum;		
		static double lastTime;
		static double interval;
		static LARGE_INTEGER startTime;
		static LARGE_INTEGER frequency;
		static bool init;

		//<summary>Private constructor</summary>
		Time();

};

#endif
