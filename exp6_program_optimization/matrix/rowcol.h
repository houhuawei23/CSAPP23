/* Matrix row and/or column summation code */

/* Size of matrices */
/* $begin rcdecl */
#define N 512
/* $end rcdecl */

/* Data types */

/* Pointer type for vectors */
typedef int *vecp_t;
/* $begin rcdecl */
/* N x N matrix */
typedef int matrix_t[N][N];

/* Vector of length N */
typedef int vector_t[N];
/* $end rcdecl */

/* Different sum/product function types */
typedef enum { COL, ROWCOL } rc_comp_t;

/* Summation function */
typedef void (*rc_fun)(matrix_t, vector_t, vector_t);

typedef struct {
    rc_fun f;
    rc_comp_t rc_type; /* What computation does it perform? */
    char *descr;
} rc_fun_rec, *rc_fun_ptr;

/* Table of functions to test.  Null terminated */
extern rc_fun_rec rc_fun_tab[];


