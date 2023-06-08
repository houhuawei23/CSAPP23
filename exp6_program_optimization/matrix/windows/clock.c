/* clock.c
 * Retrofitted to use thread-specific timers
 * and to get clock information from /proc/cpuinfo
 * (C) R. E. Bryant, 2010
 *
 */

/* When this constant is not defined, uses time stamp counter */
#define USE_POSIX 0

/* Choice to use cpu_gettime call or Intel time stamp counter directly */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <x86intrin.h>
//#include <intrinsics.h>
//#include <windows.h>
#include <time.h>
#include "clock.h"
#include <stdint.h>

/* Use x86 cycle counter */

/* Initialize the cycle counter */
static unsigned cyc_hi = 0;
static unsigned cyc_lo = 0;

/* Set *hi and *lo to the high and low order bits  of the cycle counter.
   Implementation requires assembly code to use the rdtsc instruction. */
void access_counter(unsigned *hi, unsigned *lo)
{

	long long counter;

	counter = __rdtsc();
	(*hi) = (unsigned int)(counter >> 32);
	(*lo) = (unsigned int)counter;
/*

	LARGE_INTEGER lPerformanceCount;

	QueryPerformanceCounter(&lPerformanceCount);
	(*hi) = (unsigned int)lPerformanceCount.HighPart;
	(*lo) = (unsigned int)lPerformanceCount.LowPart;
//	printf("%08X %08X\n",(*hi),(*lo));
*/
}


/* Record the current value of the cycle counter. */
void start_counter()
{
    access_counter(&cyc_hi, &cyc_lo);
}

/* Return the number of cycles since the last call to start_counter. */
double get_counter()
{
    unsigned ncyc_hi, ncyc_lo;
    unsigned hi, lo, borrow;
    double result;

    /* Get cycle counter */
    access_counter(&ncyc_hi, &ncyc_lo);

    /* Do double precision subtraction */
    lo = ncyc_lo - cyc_lo;
    borrow = cyc_lo > ncyc_lo;
    hi = ncyc_hi - cyc_hi - borrow;
    result = (double) hi * (1 << 30) * 4 + lo;
    return result;
}
void make_CPU_busy(void)
{
	volatile double old_tick,new_tick;
	start_counter();
	old_tick = get_counter();
	new_tick = get_counter();
	while (new_tick - old_tick < 1000000000)
		new_tick = get_counter();
}

//CPU的频率
double mhz(int verbose)
{
    LARGE_INTEGER lFrequency;
    LARGE_INTEGER lPerformanceCount_Start;
    LARGE_INTEGER lPerformanceCount_End;
	double mhz;
	double fTime;
	int64_t _i64StartCpuCounter;
	int64_t _i64EndCpuCounter;
    //On a multiprocessor machine, it should not matter which processor is called.
    //However, you can get different results on different processors due to bugs in
    //the BIOS or the HAL. To specify processor affinity for a thread, use the SetThreadAffinityMask function.
    HANDLE hThread=GetCurrentThread();
    SetThreadAffinityMask(hThread,0x1);

    //主板上高精度定时器的晶振频率
    //这个定时器应该就是一片8253或者8254
    //在intel ich7中集成了8254
    QueryPerformanceFrequency(&lFrequency);
//    if (verbose>0)
//    	printf("高精度定时器的晶振频率：%1.0fHz.\n",(double)lFrequency.QuadPart);

    //这个定时器每经过一个时钟周期，其计数器会+1
    QueryPerformanceCounter(&lPerformanceCount_Start);

    //RDTSC指令:获取CPU经历的时钟周期数
    _i64StartCpuCounter=__rdtsc();

    //延时长一点,误差会小一点
    //int nTemp=100000;
    //while (--nTemp);
    Sleep(200);

    QueryPerformanceCounter(&lPerformanceCount_End);

    _i64EndCpuCounter=__rdtsc();

    //f=1/T => f=计数次数/(计数次数*T)
    //这里的“计数次数*T”就是时间差
    fTime=((double)lPerformanceCount_End.QuadPart-(double)lPerformanceCount_Start.QuadPart)
        /(double)lFrequency.QuadPart;

 		mhz = (_i64EndCpuCounter-_i64StartCpuCounter)/(fTime*1000000.0);
    if (verbose>0)
    	printf("CPU频率为:%1.6fMHz.\n",mhz);
    return mhz;
}

double CPU_Factor1(void)
{
	double result;
	int i,j,k,ii,jj,kk;
	LARGE_INTEGER lStart,lEnd;
  LARGE_INTEGER lFrequency;
  HANDLE hThread;
  double fTime;

  QueryPerformanceFrequency(&lFrequency);

	ii = 43273;
	kk = 1238;
	result = 1;
	jj = 1244;

    hThread=GetCurrentThread();
    SetThreadAffinityMask(hThread,0x1);
  QueryPerformanceCounter(&lStart);
  //_asm("cpuid");
	start_counter();
	for (i=0;i<100;i++)
		for (j=0;j<1000;j++)
			for (k=0;k<1000;k++)
				kk += kk*ii+jj;

	result = get_counter();
	QueryPerformanceCounter(&lEnd);
  fTime=((double)lEnd.QuadPart-(double)lStart.QuadPart);
	printf("CPU运行时间为%f",result);
	printf("\t %f\n",fTime);
	return result;
}

double CPU_Factor(void)
{
 double frequency;
 double multiplier = 1000 * 1000 * 1000;//nano
 LARGE_INTEGER lFrequency;
 LARGE_INTEGER start,stop;
 HANDLE hThread;
 int i;
 const int gigahertz= 1000*1000*1000;
 const int known_instructions_per_loop = 27317; 

 int iterations = 100000000;
 int g = 0;
 double normal_ticks_per_second;
double ticks;
double time;
double loops_per_sec;
double instructions_per_loop;
double ratio;
double actual_freq;

 QueryPerformanceFrequency(&lFrequency);
 frequency = (double)lFrequency.QuadPart;

 hThread=GetCurrentThread();
 SetThreadAffinityMask(hThread,0x1);
 QueryPerformanceCounter(&start);
 for( i = 0; i < iterations; i++)
 {
   g++;
   g++;
   g++;
   g++;
 }
 QueryPerformanceCounter(&stop);

 //normal ticks differs from the WMI data, i.e 3125, when WMI 3201, and CPUZ 3199
 normal_ticks_per_second = frequency * 1000;
 ticks = (double)((double)stop.QuadPart - (double)start.QuadPart);
 time = (ticks * multiplier) /frequency;
 loops_per_sec = iterations / (time/multiplier);
 instructions_per_loop = normal_ticks_per_second  / loops_per_sec;

 ratio = (instructions_per_loop / known_instructions_per_loop);
 actual_freq = normal_ticks_per_second / ratio;
/* 
 actual_freq = normal_ticks_per_second / ratio;
 actual_freq = known_instructions_per_loop*iterations*multiplier/time;

	2293 = x/time;
	
	2292.599713*1191533038.809362=known_instructions_per_loop*100000000*1000
 loops_per_sec = iterations*frequency / ticks
 
 instructions_per_loop =   / loops_per_sec;
*/ 
 printf("Perf counter freq: %f\n", normal_ticks_per_second);
 printf("Loops per sec:      %f\n", loops_per_sec);
 printf("Perf counter freq div loops per sec: %f\n", instructions_per_loop);
 printf("Presumed freq: %f\n", actual_freq);
 printf("ratio: %f\n", ratio);
 printf("time=%f\n",time);
 return ratio;
}
