#include <stdio.h>
#include <stdlib.h>
//#include <random.h>
#include "rowcol.h"
#include "fcyc.h"
#include "clock.h"

#define MAX_ITER_COUNT 100

/* Define performance standards */
static struct {
  double cref;  /* Cycles taken by reference solution */
  double cbest; /* Cycles taken by our best implementation */
} cstandard[2] = 
{{7.7, 6.40}, /* Column Sum */
 {9.75, 6.60} /* Row & Column Sum */
};

/* Put in code to align matrix so that it starts on a cache block boundary.
   This makes the cache performance of the code a bit more predictable
*/

/* Words per cache block.  OK if this is an estimate as long as it
   is a multiple of the actual value
*/
#define WPB 16

int verbose = 1;
int data[N*N+WPB];
int *mstart;

typedef vector_t *row_t;

/* Reference row and column sums */
vector_t rsref, csref, rcomp, ccomp;

static void init_tests(void);
extern void make_CPU_busy(void);

static void init_tests(void)
{
    int i, j;
    int bytes_per_block = sizeof(int) * WPB;
    /* round mstart up to nearest block boundary */
    mstart = (int *)
      (((int) data + bytes_per_block-1) / bytes_per_block * bytes_per_block);
    for (i = 0; i < N; i++) {
	rsref[i] = csref[i] = 0;
    }
    for (i = 0; i < N; i++) {
	for (j = 0; j < N; j++) {
	    int val = rand();
	    mstart[i*N+j] = val;
	    rsref[i] += val;
	    csref[j] += val;
	}
    }
}


/* Test function on all values */
int test_rc(rc_fun f, FILE *rpt, rc_comp_t rc_type) {
    int i;
    int ok = 1;

    for (i = 0; i < N; i++)
	rcomp[i] = ccomp[i] = 0xDEADBEEF;
    f((row_t)mstart, rcomp, ccomp);

    for (i = 0; ok && i < N; i++) {
	if (rc_type == ROWCOL
	    && rsref[i] != rcomp[i]) {
	    ok = 0;
	    if (rpt)
		fprintf(rpt,
			"对第%d行的计算出错！正确结果是%d，但是计算得到%d\n",
			i, rsref[i], rcomp[i]);
	}
	if ((rc_type == ROWCOL || rc_type == COL)
		 && csref[i] != ccomp[i]) {
	    ok = 0;
	    if (rpt)
		fprintf(rpt,
			"对第%d列的计算出错！正确结果是%d，但是计算得到%d\n",
			i, csref[i], ccomp[i]);
	}

    }
    return ok;
}

/* Kludgy way to interface to cycle measuring code */
void do_test(int *intf)
{
  rc_fun f = (rc_fun) intf;
  f((row_t)mstart, rcomp, ccomp);
}

void time_rc(rc_fun f, rc_comp_t rc_type, char *descr, double *cycp)
{
	int i;
  int *intf = (int *) f;
  double t, cme;
  t = 0;
  if (verbose) printf("函数：%s\n", descr);
  if (test_rc(f, stdout, rc_type)) {
  	make_CPU_busy();
  	for (i=0;i<MAX_ITER_COUNT;i++)
    	t += fcyc(do_test, intf);
    t = t/MAX_ITER_COUNT;
    cme = t/(N*N);
    if (verbose) printf("  总周期数 = %.2f, 平均周期/元素 = %.2f\n",
	   t, cme);
    if (cycp)
      *cycp = cme;
  }
}

/* Compute the grade achieved by function */
static double compute_score(double cmeas, double cref, double cbest)
{
  double sbest = cref/cbest;
  double smeas = cref/cmeas;
  printf("Real score: %.3f\n", 100*((smeas-1.0)/(sbest-1.0) + 0.1));
  if (smeas < 0.1*(sbest-1)+1)
    return 0;
  if (smeas > 1.1*(sbest-1)+1)
    return 120;
  return 100*((smeas-1.0)/(sbest-1.0) + 0.1);
}

int main(int argc, char *argv[])
{
  int i;
  double cme;
  double cme_c,cme_rc;
  int EnableScore=0;
  
  if (argc == 3)
  {
  	EnableScore = 1;
  	verbose = 0;
  }
  init_tests();
  set_fcyc_clear_cache(1);  /* Set so that clears cache between runs */
  for (i = 0; rc_fun_tab[i].f != NULL; i++) {
      cme = 100.0;
      time_rc(rc_fun_tab[i].f,
	    rc_fun_tab[i].rc_type, rc_fun_tab[i].descr, &cme);
    if (i == 0)
    {
    	cme_c = cme;
    	if (EnableScore==0)
    	{
      printf("  最高\"列求和\"得分   ======================== %.0f\n",
	     compute_score(cme, cstandard[0].cref, cstandard[0].cbest));
	    }
	  }
    if (i == 1)
    {
    	cme_rc = cme;
    	if (EnableScore==0)
    	{
      printf("  最高\"行和列求和\"得分 ====================== %.0f\n",
	     compute_score(cme, cstandard[1].cref, cstandard[1].cbest));
	    }
	  }
  }
  
  if (EnableScore)
  	printf("%.2f\t %.0f\t %.2f\t %.0f\t 0\t 0\n",cme_c,compute_score(cme_c, cstandard[0].cref, cstandard[0].cbest),
  	cme_rc,compute_score(cme_rc, cstandard[1].cref, cstandard[1].cbest));
  return 0;
}
