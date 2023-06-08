/*
  Integer polynomial evaluation.
  Polynomial given by array of coefficients a[0] ... a[degree].
  Want to compute SUM(i=0,degree) a[i] * x^i
*/

/* Type declaration for a polynomial evaluation function */  
typedef int (*peval_fun)(int*, int, int);

typedef struct {
  peval_fun f;
  char *descr;
} peval_fun_rec, *peval_fun_ptr;

/* Table of polynomial functions to test.  Null terminated */
extern peval_fun_rec peval_fun_tab[];

