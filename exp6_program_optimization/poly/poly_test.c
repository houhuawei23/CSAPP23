/* Test setup for polynomial evaluation.  Do not change this. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <random.h>
#include "poly.h"
#include "cpe.h"
#include "clock.h"

double CPU_Mhz;

/* Degree for fixed evaluation */
#define FIXDEGREE 10
/* Largest degree polynomial tested */
#define MAXDEGREE 2000
static int coeff[MAXDEGREE+1];

#define MAX_ITER_COUNT 100

#define REF_CPU_MHZ 2292.6		// 这是我的处理器主频

/* Define performance standards */
static struct {
  double cref;  /* Cycles taken by reference solution */
  double cbest; /* Cycles taken by our best implementation */
} cstandard[3] =
{{4.00, 1.75}, /* CPE */
 {50, 43}, /* C(10) */
 {57,31} /* 常系数多项式计算 */
};

int coeff_const[4];

/* Should I print extra information? */
int verbose = 0;

/* Standard value for polynomial evaluation */
static int xval;

/* How many degrees should I compute reference value for? */
#define DCNT 20

/* Correct value of polynomial evaluation for range of different degrees */
/* pval[i] contains evaluation for degree MAXDEGREE-i */
static int pval[DCNT];
/* fixval contains evaluation for degree FIXDEGREE */
static int fixval;
static int fixval_const;

static void init_const_poly(void);
static void init(void);
extern int const_poly_eval(int *not_use, int not_use2, int x);
void run_fun_const(int degree);
static double compute_score(double cmeas, double cref, double cbest);
unsigned long rand1_h,rand1_l,rand_div;
void rand_step(unsigned long divv);
void GenerateRandomNumber(unsigned long divv);
extern void make_CPU_busy(void);
double run_poly_perf_test(void);

/* Reference implementation */
static int ref_poly_eval(int *a, int degree, int x)
{
    int result = 0;
    int i;
    int xpwr = 1; /* Successive powers of x */
    for (i = 0; i <= degree; i++) {
	result += a[i]*xpwr;
	xpwr   *= x;
    }
    return result;
}

/* Initialize polynomial to constant values and compute reference values */
static void init_const_poly(void)
{
    int i;

    for (i=0;i<4;i++)
    {
    	GenerateRandomNumber(90);
    	coeff_const[i] = rand_div+10;
    }

    printf("你需要修改poly.c的const_poly_eval函数，实现下面的常数多项式计算！\n");
    printf("\tresult=%d+%d*x+%d*x^2+%d*x^3\n",coeff_const[0],coeff_const[1],coeff_const[2],coeff_const[3]);

		fixval_const = ref_poly_eval(coeff_const, 3, xval);
//		printf("x=%d, fixval_const=%d\n",xval,fixval_const);

}

void test_const_poly(void)
{
	int i;
	double fix_time=0;
	int my_cal = const_poly_eval(coeff_const, 3, xval);
	if (fixval_const != my_cal)
	{
		printf("常系数多项式计算const_poly_eval实现错误（x=%d），预期结果是%d，但是计算得到的是%d\n",xval,fixval_const,my_cal);
		exit(0);
	}
	fix_time = 0;
	for (i=0;i<MAX_ITER_COUNT;i++)
		fix_time += measure_function(run_fun_const, 3);
	fix_time = fix_time / MAX_ITER_COUNT;
	    printf("  常系数多项式计算时间 = %.1f\n", fix_time);
      printf("  最高的常系数多项式计算得分 ============== %.0f\n",
	     compute_score(fix_time, cstandard[2].cref, cstandard[2].cbest));
}

/* Initialize polynomial to random values and compute reference values */
static void init(void)
{
    int i;
    xval = rand();
    for (i = 0; i <= MAXDEGREE; i++)
	coeff[i] = rand();
    for (i = 0; i < DCNT; i++)
	pval[i] = ref_poly_eval(coeff, MAXDEGREE-i, xval);
    fixval = ref_poly_eval(coeff, FIXDEGREE, xval);
}

/* Test function on standard test cases. */
int test_poly(peval_fun f, FILE *rpt) {
    int i;
    int v;
    int ok = 1;
    for (i = 0; i < DCNT; i++) {
	v = f(coeff, MAXDEGREE-i, xval);
	if (v != pval[i]) {
	    ok = 0;
	    if (rpt) {
		fprintf(rpt,
 "错误！多项式计算不对！阶=%d时，计算的值是%d，而正确值是%d\n",
			MAXDEGREE-i, v, pval[i]);
	    }
	}
    }
    v = f(coeff, FIXDEGREE, xval);
    if (v != fixval) {
	ok = 0;
	if (rpt) {
	    fprintf(rpt,
    "错误！多项式计算不对！阶=%d时，计算的值是%d，而正确值是%d\n",
		    FIXDEGREE, v, fixval);
	}
    }
    return ok;
}

/* Fit into framework of cpe measuring code */
static peval_fun pfun;

volatile int sink;
/* Run pfun for given degree */
void run_fun(int degree)
{
    sink = pfun(coeff, degree, xval);
}

volatile int sink_const;
/* Run pfun for given degree */
void run_fun_const(int degree)
{
    sink_const = const_poly_eval(coeff_const, degree, xval);
}


/* Test and measure polynomial evaluation function.  Set values
   of CPE and CFIX */
void run_poly(peval_fun f, char *descr, double *cpep, double *cfixp)
{
		int i;
		double cpe=0;
		double fix_time=0;
    pfun = f;
    printf("函数：%s\n", descr);
    if (test_poly(f, stdout)) {
    	cpe = 0;
    	for (i=0;i<MAX_ITER_COUNT;i++)
				cpe += find_cpe(run_fun, MAXDEGREE);
			cpe = cpe/MAX_ITER_COUNT;
			fix_time = 0;
			for (i=0;i<MAX_ITER_COUNT;i++)
				fix_time += measure_function(run_fun, FIXDEGREE);
			fix_time = fix_time/MAX_ITER_COUNT;
		    printf("   CPE = %.2f\tC(%d) = %.1f\n", cpe,
		   FIXDEGREE, fix_time);
	if (cpep)
	  *cpep = cpe;
	if (cfixp)
	  *cfixp = fix_time;
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

/* 产生一个0~divv-1之间的随机数，同时更新随机数种子 */
void GenerateRandomNumber(unsigned long divv)
{
	unsigned long long x = rand1_h;
	x *= 0x6AC690C5;
	x += rand1_l;

	rand1_h = (unsigned long)x;
	rand1_l = (unsigned long)(x>>32);
	if (divv==0) return;

	rand_div = rand1_h % divv;
}

int main(int argc, char *argv[])
{
  int i;
  double cpe = cstandard[0].cref;
  double cfix = cstandard[1].cref;
  verbose = 0;
  srand((unsigned int)time(NULL));

//	CPU_Factor();
//	GetCpuClock();
	printf("\t2015多项式优化实验，欢迎你！\n");
	printf("============================\n");

	if (argc == 1)
	{
		printf("使用方法：%s 学号后6位 [学号后6位] [学号后6位] ...\n",argv[0]);
		printf("你需要依据提示改写poly.c程序，实现一个常系数多项式的计算，尽可能快哦....\n");
		printf("另外，你需要改写poly.c程序，实现任意阶的多项式计算和10阶的多项式计算，要快！\n");
		return 0;
	}

	/*依据学号，初始化一个随机数发生器*/
	rand1_h = (unsigned long)atoi(argv[1]);
	rand1_l=0x29A;
	GenerateRandomNumber(0);
	for (i=2;i<argc;i++)
	{
		rand1_l = (unsigned long)atoi(argv[i]);
		GenerateRandomNumber(0);
	}

	GenerateRandomNumber(50);
	//srand(rand_div);

	//make_CPU_busy();
	//CPU_Mhz=mhz(1);
  init();
  init_const_poly();
	printf("============================\n");
	//make_CPU_busy();
	//run_poly_perf_test();
  test_const_poly();
  for (i = 0; peval_fun_tab[i].f != NULL; i++) {
  	//make_CPU_busy();
    run_poly(peval_fun_tab[i].f, peval_fun_tab[i].descr, &cpe, &cfix);
    if (i == 0)
      printf("  最高的CPE得分 =========================== %.0f\n",
	     compute_score(cpe, cstandard[0].cref, cstandard[0].cbest));
    if (i == 1)
      printf("  最高的C(10)得分 ========================= %.0f\n",
	     compute_score(cfix, cstandard[1].cref, cstandard[1].cbest));
  }
  return 0;
}



int poly_eval_perf_test(int *a, int degree, int x)
{
	int result = 0;
	int i;
	int xpwr = 1; /* Successive powers of x */
	for (i = 0; i <= degree; i++) {
		result += a[i] * xpwr;
		xpwr *= x;
	}
	return result;
}

double run_poly_perf_test(void)
{
	int i;
	double fix_time=0;
	pfun = poly_eval_perf_test;
	for (i=0;i<MAX_ITER_COUNT;i++)
		fix_time += measure_function(run_fun, FIXDEGREE);
	fix_time = fix_time/MAX_ITER_COUNT;
	printf("fix_time=%f\n",fix_time);
	return fix_time;
}
