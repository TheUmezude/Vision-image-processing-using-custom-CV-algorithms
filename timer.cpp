
// time measurement functions

#include <iostream>
#include <cmath>
#include <conio.h>

// windows functions
#include <windows.h>

using namespace std;

// user defined include files
#include "timer.h"

double high_resolution_time()
// windows high resolution time function
{
	static int init=0;
	static double pow32, count_low0, count_high0;
	double t, count_low, count_high, timer_frequency;
	LARGE_INTEGER count;

	if(init==0) {
		pow32 = pow(2.0,32); // calculate a constant, 2^32
		QueryPerformanceCounter(&count); // get initial count
		count_low0  = count.LowPart;
		count_high0 = count.HighPart;
		init=1;
	}

	// read the timer frequency
	QueryPerformanceFrequency(&count);
	timer_frequency = count.LowPart;

	// read the timer
	QueryPerformanceCounter(&count);
	count_low  = count.LowPart  - count_low0;
	count_high = count.HighPart - count_high0; 

	// calculate the time
	t = (count_low + count_high*pow32) / timer_frequency;

	return t;
}
