#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#ifdef _WIN32
#include <windows.h>
#include <intrin.h>
#else
#include <unistd.h>
#endif
#include "clock.h"

void start_counter(void);
double get_counter(void);

/*
 * Routines for using the cycle counter
 */

 /* $begin x86cyclecounter */
 /* Initialize the cycle counter */
static unsigned cyc_hi = 0;
static unsigned cyc_lo = 0;


/* Set *hi and *lo to the high and low order bits  of the cycle counter.
   Implementation requires assembly code to use the rdtsc instruction. */
void access_counter(unsigned* hi, unsigned* lo)
{
#ifdef _WIN32
	long long counter;
	counter = __rdtsc();
	(*hi) = (unsigned int)(counter >> 32);
	(*lo) = (unsigned int)counter;
#else
	asm("rdtsc; movl %%edx,%0; movl %%eax,%1"   /* Read cycle counter */
		: "=r" (*hi), "=r" (*lo)                /* and move results to */
		: /* No input */                        /* the two outputs */
		: "%edx", "%eax");
#endif
}

/* Record the current value of the cycle counter. */
void start_counter(void)
{
	access_counter(&cyc_hi, &cyc_lo);
}

/* Return the number of cycles since the last call to start_counter. */
double get_counter(void)
{
	unsigned ncyc_hi, ncyc_lo;
	unsigned hi, lo, borrow;
	double result;

	/* Get cycle counter */
	access_counter(&ncyc_hi, &ncyc_lo);

	/* Do double precision subtraction */
	lo = ncyc_lo - cyc_lo;
	borrow = lo > ncyc_lo;
	hi = ncyc_hi - cyc_hi - borrow;
	result = (double)hi * (1 << 30) * 4 + lo;
	if (result < 0) {
		printf("[%s]错误！counter返回了负值：%.0f\n", __func__, result);
	}
	return result;
}

void make_CPU_busy(void)
{
	volatile double old_tick, new_tick;
	start_counter();
	old_tick = get_counter();
	new_tick = get_counter();
	while (new_tick - old_tick < 1000000000)
		new_tick = get_counter();
}

/* $begin mhz */
/* Estimate the clock rate by measuring the cycles that elapse */
/* while sleeping for sleeptime seconds */
double mhz_full(int verbose, int sleeptime)
{
	double rate;

	if (verbose)
		printf("[%s]准备开始测量处理器性能，睡眠时间=%d秒\n", __func__, sleeptime);
	start_counter();
#ifdef _WIN32
	Sleep(sleeptime * 1000);
#else
	sleep(sleeptime);
#endif
	rate = get_counter() / (1e6 * sleeptime);
	if (verbose)
		printf("[%s]处理器主频 ~= %.1f MHz\n", __func__, rate);
	return rate;
}
/* $end mhz */

/* Version using a default sleeptime */
double mhz(int verbose)
{
	return mhz_full(verbose, 2);
}


