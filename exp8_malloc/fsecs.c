/****************************
 * High-level timing wrappers
 ****************************/
#include <stdio.h>
#include "fsecs.h"
#include "fcyc.h"
#include "clock.h"
 //#include "ftimer.h"
#include "config.h"

static double Mhz;  /* estimated CPU clock frequency */

extern int verbose; /* -v option in mdriver.c */

/*
 * init_fsecs - initialize the timing package
 */
void init_fsecs(void)
{
	Mhz = 0; /* keep gcc -Wall happy */

	if (verbose)
		printf("[%s]通过cycle计数器来测量性能.\n", __func__);

	/* set key parameters for the fcyc package */
	set_fcyc_maxsamples(200);
	set_fcyc_clear_cache(1);
	set_fcyc_epsilon(0.01);
	set_fcyc_k(3);
	Mhz = mhz(verbose > 0);
	//    printf("[%s]Your CPU is %fMhz\n",__func__, Mhz);
}

/*
 * fsecs - Return the running time of a function f (in seconds)
 */
double fsecs(fsecs_test_funct f, void* argp)
{
	double cycles = fcyc(f, argp);
	return cycles / (Mhz * 1e6);
}


