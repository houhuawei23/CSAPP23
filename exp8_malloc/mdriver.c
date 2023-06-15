/*
 * mdriver.c - CS:APP Malloc Lab Driver
 *
 * Uses a collection of trace files to tests a malloc/free/realloc
 * implementation in mm.c.
 *
 * Copyright (c) 2002, R. Bryant and D. O'Hallaron, All rights reserved.
 * May not be used, modified, or copied without permission.
 */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <assert.h>
#include <float.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#ifndef _WIN32
#include <getopt.h>
#include <unistd.h>
#else
#include "getopt.h"
#define strdup _strdup
#define unlink _unlink
#endif

#include "mm.h"
#include "memlib.h"
#include "fsecs.h"
#include "config.h"

 /**********************
  * Constants and macros
  **********************/

  /* Misc */
#define MAXLINE     1024 /* max string size */
#define HDRLINES       4 /* number of header lines in a trace file */
#define LINENUM(i) (i+5) /* cnvt trace request nums to linenums (origin 1) */
#define MAXHEAP     1024 /* maximum allowed heap size for heapcheck */
#define MAX_ITER_COUNT 100

/* Returns true if p is ALIGNMENT-byte aligned */
#define IS_ALIGNED(p)  ((((unsigned int)(p)) % ALIGNMENT) == 0)

/******************************
 * The key compound data types
 *****************************/

 /* Records the extent of each block's payload */
typedef struct range_t {
	char* lo;              /* low payload address */
	char* hi;              /* high payload address */
	struct range_t* next;  /* next list element */
} range_t;

/* Characterizes a single trace operation (allocator request) */
typedef enum { ALLOC, FREE, REALLOC, HEAPCHECK, ERROR_BUG } RequestType;
typedef struct {
	RequestType type; /* type of request */
	int index;                        /* index for free() to use later */
	int size;                         /* byte size of alloc/realloc request */
} traceop_t;

/* Holds the information for one trace file*/
typedef struct {
	int sugg_heapsize;   /* suggested heap size (unused) */
	int num_ids;         /* number of alloc/realloc ids */
	int num_ops;         /* number of distinct requests */
	int weight;          /* weight for this trace (unused) */
	traceop_t* ops;      /* array of requests */
	char** blocks;       /* array of ptrs returned by malloc/realloc... */
	size_t* block_sizes; /* ... and a corresponding array of payload sizes */
	int* allocated;         /*
			   0  : initial value
			   1  : allocated block
			   2  : freed block
			   */
	int* check; /* used to check the correctness of the heapcheck function */
} trace_t;

/*
 * Holds the params to the xxx_speed functions, which are timed by fcyc.
 * This struct is necessary because fcyc accepts only a pointer array
 * as input.
 */
typedef struct {
	trace_t* trace;
	range_t* ranges;
} speed_t;

/* Summarizes the important stats for some malloc function on some trace */
typedef struct {
	/* defined for both libc malloc and student malloc package (mm.c) */
	double ops;      /* number of ops (malloc/free/realloc) in the trace */
	int valid;       /* was the trace processed correctly by the allocator? */
	double secs;     /* number of secs needed to run the trace */

	/* defined only for the student malloc package */
	double util;     /* space utilization for this trace (always 0 for libc) */

	/* Note: secs and util are only defined if valid is true */
} stats_t;

/* Captures the output of the heapcheck function */
typedef struct {
	int request_id;
	int payload;
} block_t;

/********************
 * Global variables
 *******************/
int verbose = 1;        /* global flag for verbose output */
static int errors = 0;  /* number of errs found when running student malloc */
char msg[2 * MAXLINE];  /* for whenever we need to compose an error message */
block_t heap[MAXHEAP];  /* heap to parse output of heapcheck */

/* Directory where default tracefiles are found */
static char tracedir[MAXLINE] = TRACEDIR;

/* The filenames of the default tracefiles */
static char* default_tracefiles[] = {
	DEFAULT_TRACEFILES, NULL
};


/*********************
 * Function prototypes
 *********************/

 /* these functions manipulate range lists */
static int add_range(range_t** ranges, char* lo, int size,
	int tracenum, int opnum);
static void remove_range(range_t** ranges, char* lo);
static void clear_ranges(range_t** ranges);

/* These functions read, allocate, and free storage for traces */
static trace_t* read_trace(char* tracedir, char* filename);
static void free_trace(trace_t* trace);

/* Routines for evaluating the correctness and speed of libc malloc */
static int eval_libc_valid(trace_t* trace, int tracenum);
static void eval_libc_speed(void* ptr);

/* Routines for evaluating correctnes, space utilization, and speed
   of the student's malloc package in mm.c */
static int eval_mm_valid(trace_t* trace, int tracenum, range_t** ranges);
static double eval_mm_util(trace_t* trace, int tracenum, range_t** ranges);
static void eval_mm_speed(void* ptr);

/* Various helper routines */
static void printresults(int n, stats_t* stats);
static void usage(char* exec);
static int parse_heap(char* file, block_t* heap, int max_blocks, int tracenum, int opnum);
static void expected_blocks(trace_t* trace);
static void outputted_blocks(block_t* heap, int heapsize);

void eval_libc_speed_ref_prepare(void);
void eval_libc_speed_ref(void* ptr);

void remove_enter(char* ptr);

void remove_enter(char* ptr)
{
	int i;
	int len = (int)strlen(ptr);
	for (i = 0; i < len; i++)
		if (ptr[i] == '\r' || ptr[i] == '\n')
			ptr[i] = 0;
}

/**************
 * Main routine
 **************/
int main(int argc, char** argv)
{
	FILE* TRACE_LIST;
	char path[MAXLINE];
	char line[MAXLINE];
	//double ref_malloc_speed;
	int i;
	char c;
	char** tracefiles = NULL;  /* null-terminated array of trace file names */
	int num_tracefiles = 0;    /* the number of traces in that array */
	trace_t* trace = NULL;     /* stores a single trace file in memory */
	range_t* ranges = NULL;    /* keeps track of block extents for one trace */
	stats_t* libc_stats = NULL;/* libc stats for each trace */
	stats_t* mm_stats = NULL;  /* mm (i.e. student) stats for each trace */
	speed_t speed_params;      /* input parameters to the xx_speed routines */

	//int team_check = 1;  /* If set, check team structure (reset by -a) */
	int run_libc = 0;    /* If set, run libc malloc (set by -l) */
	int autograder = 0;  /* If set, emit summary info for autograder (-g) */

	/* temporaries used to compute the performance index */
	double secs, ops, util, avg_mm_util, avg_mm_throughput, p1, p2, perfindex;
	int numcorrect;

	/*
	 * Read and interpret the command line arguments
	 */
	while ((c = getopt(argc, argv, "f:t:hvVgl")) != EOF) {
		switch (c) {
		case 'g': /* Generate summary info for the autograder */
			autograder = 1;
			break;
		case 'f': /* Use one specific trace file only (relative to curr dir) */
			num_tracefiles = 1;
			if ((tracefiles = (char**)realloc(tracefiles, 2 * sizeof(char*))) == NULL) {
				printf("[%s]realloc失败\n", __func__);
			}
			strcpy(tracedir, "./");
			tracefiles[0] = strdup(optarg);
			tracefiles[1] = NULL;
			break;
		case 't': /* Directory where the traces are located */
			if (num_tracefiles == 1) /* ignore if -f already encountered */
				break;
			strcpy(tracedir, optarg);
			if (tracedir[strlen(tracedir) - 1] != '/')
				strcat(tracedir, "/"); /* path always ends with "/" */
			break;
			//case 'a': /* Don't check team structure */
			//	team_check = 0;
			//	break;
		case 'l': /* Run libc malloc */
			run_libc = 1;
			break;
		case 'v': /* Print per-trace performance breakdown */
			verbose = 1;
			break;
		case 'V': /* Be more verbose than -v */
			verbose = 2;
			break;
		case 'h': /* Print this message */
			usage(argv[0]);
			exit(0);
		default:
			usage(argv[0]);
			exit(1);
		}
	}

	/*
	 * If no -f command line arg, then use the entire set of tracefiles
	 * defined in default_traces[]
	 */
	if (tracefiles == NULL) {

		strcpy(path, tracedir);
		strcat(path, "TRACE_LIST.txt");
		TRACE_LIST = fopen(path, "r");
		if (TRACE_LIST == NULL)
		{
			printf("[%s]没有找到%s文件，使用缺省的trace文件(%s)\n", __func__, path, tracedir);
			tracefiles = default_tracefiles;
			num_tracefiles = sizeof(default_tracefiles) / sizeof(char*) - 1;
		}
		else
		{
			num_tracefiles = 0;
			while (fgets(line, MAXLINE, TRACE_LIST))
			{
				if (line[0] == '#' || line[0] == '/' || line[0] == '\n' || line[0] == '\r') continue;
				tracefiles = realloc(tracefiles, (num_tracefiles + 2) * sizeof(char*));
				remove_enter(line);
				tracefiles[num_tracefiles] = strdup(line);
				tracefiles[num_tracefiles + 1] = NULL;
				num_tracefiles++;
			}

			fclose(TRACE_LIST);
			printf("[%s]num_tracefiles=%d\n", __func__, num_tracefiles);
			for (i = 0; i < num_tracefiles; i++)
				printf("[%2d]%s\n", i, tracefiles[i]);
		}
	}

	/* Initialize the timing package */
	init_fsecs();
	/*
			ref_malloc_speed = 0;
		eval_libc_speed_ref_prepare();
		ref_malloc_speed = fsecs(eval_libc_speed_ref,&speed_params);

		printf("ref_malloc_speed=%f\n",ref_malloc_speed);
	//    init_fsecs();
	*/
	/*
	 * Optionally run and evaluate the libc malloc package
	 */
	if (run_libc) {
		if (verbose > 1)
			printf("\n[%s]检测libc库函数malloc\n", __func__);

		/* Allocate libc stats array, with one stats_t struct per tracefile */
		libc_stats = (stats_t*)calloc(num_tracefiles, sizeof(stats_t));
		if (libc_stats == NULL)
			printf("[%s]libc_stats calloc失败\n", __func__);

		/* Evaluate the libc malloc package using the K-best scheme */
		for (i = 0; i < num_tracefiles; i++) {
			trace = read_trace(tracedir, tracefiles[i]);
			libc_stats[i].ops = trace->num_ops;
			if (verbose > 1)
				printf("[%s]检测libc malloc的正确性, ", __func__);
			libc_stats[i].valid = eval_libc_valid(trace, i);
			if (libc_stats[i].valid) {
				speed_params.trace = trace;
				if (verbose > 1)
					printf("和性能\n");
				libc_stats[i].secs = fsecs(eval_libc_speed, &speed_params);
			}
			free_trace(trace);
		}

		/* Display the libc results in a compact table */
		if (verbose) {
			printf("\n[%s]libc malloc的检测结果:\n", __func__);
			printresults(num_tracefiles, libc_stats);
		}
	}

	/*
	 * Always run and evaluate the student's mm package
	 */
	if (verbose > 1)
		printf("\n[%s]检测mm文件中的malloc\n", __func__);

	/* Allocate the mm stats array, with one stats_t struct per tracefile */
	mm_stats = (stats_t*)calloc(num_tracefiles, sizeof(stats_t));
	if (mm_stats == NULL)
		printf("[%s]mm_stats calloc失败\n", __func__);

	/* Initialize the simulated memory system in memlib.c */
	mem_init();

	/* Evaluate student's mm malloc package using the K-best scheme */
	for (i = 0; i < num_tracefiles; i++) {
		trace = read_trace(tracedir, tracefiles[i]);
		mm_stats[i].ops = trace->num_ops;
		if (verbose > 1)
			printf("[%s]检测mm_malloc的正确性, ", __func__);
		mm_stats[i].valid = eval_mm_valid(trace, i, &ranges);
		if (mm_stats[i].valid) {
			if (verbose > 1)
				printf("效率, ");
			mm_stats[i].util = eval_mm_util(trace, i, &ranges);
			speed_params.trace = trace;
			speed_params.ranges = ranges;
			if (verbose > 1)
				printf("和性能.\n");
			//		mm_stats[i].secs = 0;
			//			for (ii=0;ii<MAX_ITER_COUNT;ii++)
			mm_stats[i].secs = fsecs(eval_mm_speed, &speed_params);
			//	    mm_stats[i].secs = mm_stats[i].secs/MAX_ITER_COUNT;
		}

		free_trace(trace);
	}

	/* Display the mm results in a compact table */
	if (verbose) {
		printf("\n[%s]mm malloc的检测结果:\n", __func__);
		printresults(num_tracefiles, mm_stats);
		printf("\n");
	}

	/*
	 * Accumulate the aggregate statistics for the student's mm package
	 */
	secs = 0;
	ops = 0;
	util = 0;
	numcorrect = 0;
	for (i = 0; i < num_tracefiles; i++) {
		secs += mm_stats[i].secs;
		ops += mm_stats[i].ops;
		util += mm_stats[i].util;
		if (mm_stats[i].valid)
			numcorrect++;
	}
	avg_mm_util = util / num_tracefiles;

	/*
	 * Compute and print the performance index
	 */
	if (errors == 0) {
		avg_mm_throughput = ops / secs;

		p1 = UTIL_WEIGHT * avg_mm_util;
		if (avg_mm_throughput > AVG_LIBC_THRUPUT) {
			p2 = (double)(1.0 - UTIL_WEIGHT);
		}
		else {
			p2 = ((double)(1.0 - UTIL_WEIGHT)) *
				(avg_mm_throughput / AVG_LIBC_THRUPUT);
		}

		perfindex = (p1 + p2) * 100.0;
		printf("[%s]评分 = %.0f (效率%d) + %.0f (性能%d) = %.0f/100\n", __func__,
			p1 * 100, (int)(UTIL_WEIGHT * 100),
			p2 * 100, (int)(100 - UTIL_WEIGHT * 100),
			perfindex);

	}
	else { /* There were errors */
		perfindex = 0.0;
		printf("[%s]程序因为%d个错误而终止！\n", __func__, errors);
	}

	if (autograder) {
		printf("[%s]正确:%d\n", __func__, numcorrect);
		printf("[%s]评分:%.0f\n", __func__, perfindex);
	}

	exit(0);
}


/*****************************************************************
 * The following routines manipulate the range list, which keeps
 * track of the extent of every allocated block payload. We use the
 * range list to detect any overlapping allocated blocks.
 ****************************************************************/

 /*
  * add_range - As directed by request opnum in trace tracenum,
  *     we've just called the student's mm_malloc to allocate a block of
  *     size bytes at addr lo. After checking the block for correctness,
  *     we create a range struct for this block and add it to the range list.
  */
static int add_range(range_t** ranges, char* lo, int size,
	int tracenum, int opnum)
{
	char* hi = lo + size - 1;
	range_t* p;

	assert(size > 0);

	/* Payload addresses must be ALIGNMENT-byte aligned */
	if (!IS_ALIGNED(lo)) {
		printf("[%s]Payload的地址(%p)没有对齐到%d字节的边界\n", __func__, lo, ALIGNMENT);
		return 0;
	}

	/* The payload must lie within the extent of the heap */
	if ((lo < (char*)mem_heap_lo()) || (lo > (char*)mem_heap_hi()) ||
		(hi < (char*)mem_heap_lo()) || (hi > (char*)mem_heap_hi())) {
		printf("[%s]Payload (%p:%p)处在堆范围之外(%p:%p)\n", __func__, lo, hi, mem_heap_lo(), mem_heap_hi());
		return 0;
	}

	/* The payload must not overlap any other payloads */
	for (p = *ranges; p != NULL; p = p->next) {
		if ((lo >= p->lo && lo <= p->hi) ||
			(hi >= p->lo && hi <= p->hi)) {
			printf("[%s]Payload (%p:%p)与另外的payload重叠了(%p:%p)\n", __func__, lo, hi, p->lo, p->hi);
			return 0;
		}
	}

	/*
	 * Everything looks OK, so remember the extent of this block
	 * by creating a range struct and adding it the range list.
	 */
	if ((p = (range_t*)malloc(sizeof(range_t))) == NULL)
		printf("[%s]malloc失败\n", __func__);
	p->next = *ranges;
	p->lo = lo;
	p->hi = hi;
	*ranges = p;
	return 1;
}

/*
 * remove_range - Free the range record of block whose payload starts at lo
 */
static void remove_range(range_t** ranges, char* lo)
{
	range_t* p;
	range_t** prevpp = ranges;

	for (p = *ranges; p != NULL; p = p->next) {
		if (p->lo == lo) {
			*prevpp = p->next;
			// int size = p->hi - p->lo + 1;
			free(p);
			break;
		}
		prevpp = &(p->next);
	}
}

/*
 * clear_ranges - free all of the range records for a trace
 */
static void clear_ranges(range_t** ranges)
{
	range_t* p;
	range_t* pnext;

	for (p = *ranges; p != NULL; p = pnext) {
		pnext = p->next;
		free(p);
	}
	*ranges = NULL;
}


/**********************************************
 * The following routines manipulate tracefiles
 *********************************************/

 /*
  * read_trace - read a trace file and store it in memory
  */
static trace_t* read_trace(char* tracedir, char* filename)
{
	FILE* tracefile;
	trace_t* trace;
	char type[MAXLINE];
	char path[MAXLINE];
	int index, size;
	int max_index = 0;
	int op_index;
	int i;
	int line_count = 0;
	int retval;

	if (verbose > 1)
		printf("[%s]读取trace文件: %s\n", __func__, filename);

	/* Allocate the trace record */
	if ((trace = (trace_t*)malloc(sizeof(trace_t))) == NULL)
		printf("[%s]malloc trace失败\n", __func__);

	/* Read the trace file header */
	strcpy(path, tracedir);
	strcat(path, filename);
	if ((tracefile = fopen(path, "r")) == NULL) {
		printf("[%s]不能打开文件 %s\n", __func__, path);
	}
	retval = fscanf(tracefile, "%d", &(trace->sugg_heapsize)); /* not used */
	if (retval == 0)
	{
		printf("[%s]读取trace文件格式错误：没有sugg_heapsize\n", __func__);
	}
	retval = fscanf(tracefile, "%d", &(trace->num_ids));
	if (retval == 0)
	{
		printf("[%s]读取trace文件格式错误：没有num_ids\n", __func__);
	}
	retval = fscanf(tracefile, "%d", &(trace->num_ops));
	if (retval == 0)
	{
		printf("[%s]读取trace文件格式错误：没有num_ops\n", __func__);
	}
	retval = fscanf(tracefile, "%d", &(trace->weight));        /* not used */
	if (retval == 0)
	{
		printf("[%s]读取trace文件格式错误：没有weight\n", __func__);
	}

	line_count = 4;
	/* We'll store each request line in the trace in this array */
	if ((trace->ops =
		(traceop_t*)malloc(trace->num_ops * sizeof(traceop_t))) == NULL)
		printf("[%s]malloc trace->ops失败\n", __func__);

	/* We'll keep an array of pointers to the allocated blocks here... */
	if ((trace->blocks =
		(char**)malloc(trace->num_ids * sizeof(char*))) == NULL)
		printf("[%s]malloc trace->blocks失败", __func__);
	memset(trace->blocks, 0, trace->num_ids * sizeof(char*));

	/* ... along with the corresponding byte sizes of each block */
	if ((trace->block_sizes =
		(size_t*)malloc(trace->num_ids * sizeof(size_t))) == NULL)
		printf("[%s]malloc trace->block_sizes失败", __func__);

	/* ... along with flags for the heapcheck function */
	if ((trace->allocated =
		(int*)malloc(trace->num_ids * sizeof(int))) == NULL)
		printf("[%s]malloc trace->allocated失败", __func__);
	/* initialize allocated to 0 */
	for (i = 0; i < trace->num_ids; i++)
		trace->allocated[i] = 0;

	if ((trace->check =
		(int*)malloc(trace->num_ids * sizeof(int))) == NULL)
		printf("[%s]malloc trace->check失败", __func__);

	/* read every request line in the trace file */
	index = 0;
	op_index = 0;
	while (fscanf(tracefile, "%s", type) != EOF) {
		line_count++;
		switch (type[0]) {
		case 'a':
			if (2 != fscanf(tracefile, "%u %u", &index, &size)) {
				printf("[%s]%d 行,allocation需要两个参数\n", __func__, line_count);
			}
			trace->ops[op_index].type = ALLOC;
			trace->ops[op_index].index = index;
			trace->ops[op_index].size = size;
			max_index = (index > max_index) ? index : max_index;

			// workaround realloc bug
			trace->blocks[index] = (char *)1;
			break;
		case 'r':
			if (2 != fscanf(tracefile, "%u %u", &index, &size)) {
				printf("[%s]%d 行,relloc需要两个参数\n", __func__, line_count);
			}
			if (trace->blocks[index] == (char*)1)
			{
				trace->ops[op_index].type = REALLOC;
				trace->blocks[index] = (char*)1;
			}
			else
			{
				// workaround realloc bug
				if (verbose > 1)
					printf("[%s]%d 行,尝试realloc一个未初始化的指针(%d)，修改为alloc\n", __func__, line_count, index);
				trace->ops[op_index].type = ALLOC;
			}
			trace->ops[op_index].index = index;
			trace->ops[op_index].size = size;
			max_index = (index > max_index) ? index : max_index;
			break;
		case 'c':
			trace->ops[op_index].type = HEAPCHECK;
			break;
		case 'f':
			if (1 != fscanf(tracefile, "%ud", &index)) {
				printf("[%s]%d 行,free需要一个参数\n", __func__, line_count);
			}
			// workaround free -1 bug
			if (index == -1)
			{
				if (verbose > 1)
					printf("[%s]%d 行,free一个-1指针，跳过\n", __func__, line_count);
				trace->ops[op_index].type = ERROR_BUG;
			}
			else
				trace->ops[op_index].type = FREE;
			trace->ops[op_index].index = index;
			break;
		default:
			printf("[%s]trace文件[%s,%d行]中错误的类型字符(%c)\n", __func__, path, line_count, type[0]);
			exit(1);
		}
		op_index++;

	}
	fclose(tracefile);
	assert(max_index == trace->num_ids - 1);
	assert(trace->num_ops == op_index);

	return trace;
}

/*
 * free_trace - Free the trace record and the three arrays it points
 *              to, all of which were allocated in read_trace().
 */
static void free_trace(trace_t* trace)
{
	free(trace->ops);         /* free the three arrays... */
	free(trace->blocks);
	free(trace->block_sizes);
	free(trace);              /* and the trace record itself... */
}

/**********************************************************************
 * The following functions evaluate the correctness, space utilization,
 * and throughput of the libc and mm malloc packages.
 **********************************************************************/

 /*
  * eval_mm_valid - Check the mm malloc package for correctness
  */
static int eval_mm_valid(trace_t* trace, int tracenum, range_t** ranges)
{
	int i, j;
	int index;
	int size;
	int oldsize;
	char* newp;
	char* oldp;
	char* p;
	char tmpfile[L_tmpnam];
	//int fd;
	//int stdoutold;
	int heapsize;
	int request_id;
	int payload;
	char the_char;

	/* Reset the heap and free any records in the range list */
	mem_reset_brk();
	clear_ranges(ranges);

	/* Call the mm package's init function */
	if (mm_init() < 0) {
		printf("[%s]mm_init失败[tracenum=%d]\n", __func__, tracenum);
		return 0;
	}

	/* Interpret each operation in the trace in order */
	for (i = 0; i < trace->num_ops; i++) {
		index = trace->ops[i].index;
		size = trace->ops[i].size;

		switch (trace->ops[i].type) {

		case ALLOC: /* mm_malloc */
			//printf("ALLOC\n");

		/* Call the student's malloc */
			if ((p = mm_malloc(size)) == NULL) {
				printf("[%s]mm_malloc失败[tracenum=%d,i=%d]\n", __func__, tracenum, i);
				return 0;
			}

			/*
			 * Test the range of the new block for correctness and add it
			 * to the range list if OK. The block must be  be aligned properly,
			 * and must not overlap any currently allocated block.
			 */
			if (add_range(ranges, p, size, tracenum, i) == 0)
				return 0;

			/* ADDED: cgw
			 * fill range with low byte of index.  This will be used later
			 * if we realloc the block and wish to make sure that the old
			 * data was copied to the new block
			 */
			memset(p, index & 0xFF, size);

			/* Remember region */
			trace->blocks[index] = p;
			trace->block_sizes[index] = size;
			trace->allocated[index] = 1;
			break;

		case REALLOC: /* mm_realloc */
			//printf("REALLOC\n");

		/* Call the student's realloc */
			oldp = trace->blocks[index];
			if ((newp = (char*)mm_realloc(oldp, size)) == NULL) {
				printf("[%s]mm_realloc失败[tracenum=%d,i=%d]\n", __func__, tracenum, i);
				return 0;
			}

			/* Remove the old region from the range list */
			remove_range(ranges, oldp);

			/* Check new block for correctness and add it to range list */
			if (add_range(ranges, newp, size, tracenum, i) == 0)
				return 0;

			/* ADDED: cgw
			 * Make sure that the new block contains the data from the old
			 * block and then fill in the new block with the low order byte
			 * of the new index
			 */
			oldsize = (int)trace->block_sizes[index];
			if (size < oldsize) oldsize = size;

			the_char = (index & 0xFF);
			for (j = 0; j < oldsize; j++) {
				the_char = index & 0xFF;
				if (newp[j] != the_char) {
					printf("[%s]mm_realloc没有保存旧块的数据[tracenum = % d, i = % d]\n", __func__, tracenum, i);
					return 0;
				}
			}
			memset(newp, index & 0xFF, size);

			/* Remember region */
			trace->blocks[index] = newp;
			trace->block_sizes[index] = size;
			//printf("\nrealloc finish\n");
			break;
		case HEAPCHECK:
			//printf("HEAPCHECK\n");
	  /* check the output of the mm_heapcheck routine */
	  /* for each allocated block that hasn't been freed yet,
		 the mm_heapcheck function should print a line
		 in the following format

		 request_id payload_size stuff_for_debugging_not_used

	   */

	   /* redirect stdout to a temporary file */
	   //fd = open(tmpnam(tmpfile),O_RDWR|O_CREAT);
	   //if (fd < 0)
	   //  printf("[%s]不能为heapcheck创建临时文件\n",__func__);
	   //fflush(stdout);
	   //stdoutold = dup(1);
	   //dup2(fd,1);
	   //
	   ///* Call the student's heapcheck */
	   //mm_heapcheck();
	   //fflush(stdout);
	   //close(fd);
	   //close(1);
	   //
	   ///* fix stdout */
	   //dup2(stdoutold, 1);
	   //close(stdoutold);

	   /* parse the output of the heapcheck function */
			heapsize = parse_heap(tmpfile, heap, i, tracenum, i);

			/* initialize trace for scanning */
			for (j = 0; j < trace->num_ids; j++)
				trace->check[j] = 0;

			for (j = 0; j < heapsize; j++) {
				request_id = heap[j].request_id;
				payload = heap[j].payload;

				if (((request_id) > trace->num_ids) || (trace->allocated[request_id] == 0)) {
					printf("[%s]%d 不是一个有效的request_id[tracenum=%d,i=%d]\n", __func__, request_id, tracenum, i);
					expected_blocks(trace);
					outputted_blocks(heap, heapsize);
					return 0;
				}

				if (trace->allocated[request_id] == 2) {
					printf("[%s]输出块 %d 已经被释放了[tracenum=%d,i=%d]\n", __func__, request_id, tracenum, i);
					expected_blocks(trace);
					outputted_blocks(heap, heapsize);
					return 0;
				}

				if (trace->check[request_id] == 1) {
					printf("[%s]块 %d 被输出了两次[tracenum=%d,i=%d]\n", __func__, request_id, tracenum, i);
					expected_blocks(trace);
					outputted_blocks(heap, heapsize);
					return 0;
				}

				if (trace->block_sizes[request_id] != (size_t)payload) {
					printf("[%s]块 %d 的大小不对%d, 应该是 %zu[tracenum=%d,i=%d]\n", __func__, request_id, payload, trace->block_sizes[request_id], tracenum, i);
					expected_blocks(trace);
					outputted_blocks(heap, heapsize);
					return 0;
				}

				trace->check[request_id] = 1;
			}

			for (j = 0; j < trace->num_ids; j++)
				if ((trace->allocated[j] == 1) && (trace->check[j] == 0))
				{
					printf("[%s]块 %d 不能被heapcheck输出[tracenum=%d,i=%d]\n", __func__, j, tracenum, i);
					expected_blocks(trace);
					outputted_blocks(heap, heapsize);
					return 0;
				}


			break;

		case FREE: /* mm_free */
			//printf("FREE\n");

		/* Remove region from list and call student's free function */
			p = trace->blocks[index];
			remove_range(ranges, p);
			mm_free(p);

			/* mark the appropriate block as freed */
			trace->allocated[index] = 2;

			break;
		case ERROR_BUG:
			break;
		default:
			printf("[%s]没有request类型\n", __func__);
		}

	}

	/* As far as we know, this is a valid malloc package */
	return 1;
}

/*
 * eval_mm_util - Evaluate the space utilization of the student's package
 *   The idea is to remember the high water mark "hwm" of the heap for
 *   an optimal allocator, i.e., no gaps and no internal fragmentation.
 *   Utilization is the ratio hwm/heapsize, where heapsize is the
 *   size of the heap in bytes after running the student's malloc
 *   package on the trace. Note that our implementation of mem_sbrk()
 *   doesn't allow the students to decrement the brk pointer, so brk
 *   is always the high water mark of the heap.
 *
 */
static double eval_mm_util(trace_t* trace, int tracenum, range_t** ranges)
{
	int i;
	int index;
	int size, newsize, oldsize;
	int max_total_size = 0;
	int total_size = 0;
	char* p;
	char* newp, * oldp;

	/* initialize the heap and the mm malloc package */
	mem_reset_brk();
	if (mm_init() < 0)
		printf("[%s]mm_init失败\n", __func__);

	for (i = 0; i < trace->num_ops; i++) {
		switch (trace->ops[i].type) {

		case ALLOC: /* mm_alloc */
			index = trace->ops[i].index;
			size = trace->ops[i].size;

			if ((p = mm_malloc(size)) == NULL)
				printf("[%s]mm_malloc失败\n", __func__);

			/* Remember region and size */
			trace->blocks[index] = p;
			trace->block_sizes[index] = size;

			/* Keep track of current total size
			 * of all allocated blocks */
			total_size += size;

			/* Update statistics */
			max_total_size = (total_size > max_total_size) ?
				total_size : max_total_size;
			break;

		case REALLOC: /* mm_realloc */
			index = trace->ops[i].index;
			newsize = trace->ops[i].size;
			oldsize = (int)trace->block_sizes[index];

			oldp = trace->blocks[index];
			if ((newp = (char*)mm_realloc(oldp, newsize)) == NULL)
				printf("[%s]mm_realloc失败\n", __func__);

			/* Remember region and size */
			trace->blocks[index] = newp;
			trace->block_sizes[index] = newsize;

			/* Keep track of current total size
			 * of all allocated blocks */
			total_size += (newsize - oldsize);

			/* Update statistics */
			max_total_size = (total_size > max_total_size) ?
				total_size : max_total_size;
			break;

		case HEAPCHECK:
			break;

		case FREE: /* mm_free */
			index = trace->ops[i].index;
			size = (int)trace->block_sizes[index];
			p = trace->blocks[index];

			mm_free(p);

			/* Keep track of current total size
			 * of all allocated blocks */
			total_size -= size;

			break;
		case ERROR_BUG:
			break;
		default:
			printf("[%s]没有request类型\n", __func__);

		}
	}

	return ((double)max_total_size / (double)mem_heapsize());
}


/*
 * eval_mm_speed - This is the function that is used by fcyc()
 *    to measure the running time of the mm malloc package.
 */
static void eval_mm_speed(void* ptr)
{
	int i, index, size, newsize;
	char* p, * newp, * oldp, * block;
	trace_t* trace = ((speed_t*)ptr)->trace;

	/* Reset the heap and initialize the mm package */
	mem_reset_brk();
	if (mm_init() < 0)
		printf("[%s]mm_init失败\n", __func__);

	/* Interpret each trace request */
	for (i = 0; i < trace->num_ops; i++)
		switch (trace->ops[i].type) {

		case ALLOC: /* mm_malloc */
			index = trace->ops[i].index;
			size = trace->ops[i].size;
			if ((p = mm_malloc(size)) == NULL)
				printf("[%s]mm_malloc失败\n", __func__);
			trace->blocks[index] = p;
			break;

		case REALLOC: /* mm_realloc */
			index = trace->ops[i].index;
			newsize = trace->ops[i].size;
			oldp = trace->blocks[index];
			if ((newp = (char*)mm_realloc(oldp, newsize)) == NULL)
				printf("[%s]mm_realloc失败\n", __func__);
			trace->blocks[index] = newp;
			break;

		case HEAPCHECK:
			break;

		case FREE: /* mm_free */
			index = trace->ops[i].index;
			block = trace->blocks[index];
			mm_free(block);
			break;
		case ERROR_BUG:
			break;
		default:
			printf("[%s]没有request类型\n", __func__);
		}
}

/*
 * eval_libc_valid - We run this function to make sure that the
 *    libc malloc can run to completion on the set of traces.
 *    We'll be conservative and terminate if any libc malloc call fails.
 *
 */
static int eval_libc_valid(trace_t* trace, int tracenum)
{
	int i, newsize;
	char* p, * newp, * oldp;

	for (i = 0; i < trace->num_ops; i++) {
		switch (trace->ops[i].type) {

		case ALLOC: /* malloc */
			if ((p = malloc(trace->ops[i].size)) == NULL) {
				printf("[%s]libc malloc失败[tracenum=%d,i=%d]\n", __func__, tracenum, i);
			}
			trace->blocks[trace->ops[i].index] = p;
			break;

		case REALLOC: /* realloc */
			newsize = trace->ops[i].size;
			oldp = trace->blocks[trace->ops[i].index];
			if ((newp = (char*)realloc(oldp, newsize)) == NULL) {
				printf("[%s]libc realloc失败[tracenum=%d,i=%d]\n", __func__, tracenum, i);
			}
			trace->blocks[trace->ops[i].index] = newp;
			break;

		case HEAPCHECK:
			break;

		case FREE: /* free */
			free(trace->blocks[trace->ops[i].index]);
			break;
		case ERROR_BUG:
			break;
		default:
			printf("[%s]无效的操作类型[%d][tracenum=%d,i=%d]\n", __func__, trace->ops[i].type, tracenum, i);
		}
	}

	return 1;
}

/*
 * eval_libc_speed - This is the function that is used by fcyc() to
 *    measure the running time of the libc malloc package on the set
 *    of traces.
 */
static void eval_libc_speed(void* ptr)
{
	int i;
	int index, size, newsize;
	char* p, * newp, * oldp, * block;
	trace_t* trace = ((speed_t*)ptr)->trace;

	for (i = 0; i < trace->num_ops; i++) {
		switch (trace->ops[i].type) {
		case ALLOC: /* malloc */
			index = trace->ops[i].index;
			size = trace->ops[i].size;
			if ((p = malloc(size)) == NULL)
				printf("[%s]malloc失败\n", __func__);
			trace->blocks[index] = p;
			break;

		case REALLOC: /* realloc */
			index = trace->ops[i].index;
			newsize = trace->ops[i].size;
			oldp = trace->blocks[index];
			if ((newp = (char*)realloc(oldp, newsize)) == NULL)
				printf("[%s]realloc失败\n", __func__);

			trace->blocks[index] = newp;
			break;

		case HEAPCHECK:
			break;

		case FREE: /* free */
			index = trace->ops[i].index;
			block = trace->blocks[index];
			free(block);
			break;
		case ERROR_BUG:
			break;
		}
	}
}

unsigned int rand1_h, rand1_l, rand_div;
/* 产生一个0~divv-1之间的随机数，同时更新随机数种子 */
void GenerateRandomNumber(unsigned int divv)
{
	unsigned long long x = rand1_h;
	x *= 0x6AC690C5;
	x += rand1_l;

	rand1_h = (unsigned int)x;
	rand1_l = (unsigned int)(x >> 32);
	if (divv == 0) return;

	rand_div = rand1_h % divv;
}

#define MAX_BENCHMARK_MALLOC_TIMES 10000

struct ref_malloc_trace_struct
{
	int index;
	int size;
	char* p;
	char op;
} ref_malloc_trace[2 * MAX_BENCHMARK_MALLOC_TIMES];
/*
 * eval_libc_speed_ref_prepare - This is the function that is used by fcyc() to
 *    measure the running time of the libc malloc package on the set
 *    of traces.
 */
void eval_libc_speed_ref_prepare(void)
{
	int i, malloc_count, free_count, j, last_free_index;
	char free_flag[MAX_BENCHMARK_MALLOC_TIMES];

	malloc_count = 0;
	free_count = 0;
	last_free_index = 0;

	rand1_h = 0x11223344;
	rand1_l = 0x55667788;

	GenerateRandomNumber(0);

	memset(ref_malloc_trace, 0, sizeof(ref_malloc_trace));
	memset(free_flag, 0, sizeof(free_flag));
	i = 0;
	while (1)
	{
		GenerateRandomNumber(10);
		//printf("rand_div=%d malloc_count=%d free_count=%d\n",rand_div,malloc_count,free_count);
		if (rand_div >= 6 && malloc_count < MAX_BENCHMARK_MALLOC_TIMES)
		{
			// malloc
			ref_malloc_trace[i].index = malloc_count;
			ref_malloc_trace[i].op = 10;
			free_flag[malloc_count] = 10;
			GenerateRandomNumber(100000);
			ref_malloc_trace[i].size = rand_div + 1;
			//printf("a index=%02d size=%d\n",malloc_count,rand_div+1);
			malloc_count++;
			i++;
		}

		if (rand_div <= 4 && free_count < MAX_BENCHMARK_MALLOC_TIMES)
		{
			// free
			for (j = last_free_index; j < malloc_count; j++)
				if (free_flag[j] == 10)
				{
					//printf("f index=%02d\n",j);
					last_free_index = j;
					ref_malloc_trace[i].index = j;
					ref_malloc_trace[i].op = 20;
					free_flag[j] = 20;
					i++;
					free_count++;
					break;
				}
		}

		if (malloc_count == MAX_BENCHMARK_MALLOC_TIMES && free_count == MAX_BENCHMARK_MALLOC_TIMES)
			break;
	}
}

void eval_libc_speed_ref(void* ptr)
{
	int i;

	for (i = 0; i < 2 * MAX_BENCHMARK_MALLOC_TIMES; i++)
	{
		if (ref_malloc_trace[i].op == 10)
		{
			//printf("%02d a %d\n",ref_malloc_trace[i].index,ref_malloc_trace[i].size);
			ref_malloc_trace[ref_malloc_trace[i].index].p = (char*)malloc(ref_malloc_trace[i].size);
		}
		if (ref_malloc_trace[i].op == 20)
		{
			//printf("%02d f\n",ref_malloc_trace[i].index);
			free(ref_malloc_trace[ref_malloc_trace[i].index].p);
		}
	}
}

/*************************************
 * Some miscellaneous helper routines
 ************************************/


 /*
  * printresults - prints a performance summary for some malloc package
  */
static void printresults(int n, stats_t* stats)
{
	int i;
	double secs = 0;
	double ops = 0;
	double util = 0;

	/* Print the individual results for each trace */
	printf("[%s] +---------------------------------------------------------+\n", __func__);
	printf("[%s] | trace | 正确性 | 效率 | 操作次数 | 时间(毫秒) |   Kops  |\n", __func__);
	printf("[%s] +---------------------------------------------------------+\n", __func__);
	for (i = 0; i < n; i++) {
		if (stats[i].valid) {
			printf("[%s] | %5d | %6s | %3.0f%% | %8.0f | %10.6f |  %6.0f |\n", __func__,
				i,
				"yes",
				stats[i].util * 100.0,
				stats[i].ops,
				stats[i].secs * 1000,
				(stats[i].ops / 1e3) / stats[i].secs);
			secs += stats[i].secs;
			ops += stats[i].ops;
			util += stats[i].util;
		}
		else {
			printf("[%s] | %5d |     no |   -  |     -    |     -      |    -    |\n", __func__, i);
		}
	}

	/* Print the aggregate results for the set of traces */
	if (errors == 0) {
		printf("[%s] | 累计  |        | %3.0f%% | %8.0f | %10.6f |  %6.0f |\n\n", __func__,
			(util / n) * 100.0,
			ops,
			secs * 1000,
			(ops / 1e3) / secs);
	}
	else {
		printf("%12s%6s%10s%12s%6s\n",
			"累计        ",
			"-",
			"-",
			"-",
			"-");
	}

}

/*
 * parse_heapcheck - Parse the output of the heapcheck into an internal data structure
 * returns -1 on error, 0 on success
 */
static int parse_heap(char* file, block_t* heap, int max_blocks, int tracenum, int opnum)
{
	FILE* fp;
	char line[MAXLINE];
	char str[MAXLINE];
	int i = 0;
	int request_id;
	int payload;

	fp = fopen(file, "r");
	if (fp == NULL)
		printf("[%s]从临时文件中读取失败\n", __func__);

	while ((fgets(line, MAXLINE, fp) != NULL)) {
		if (sscanf(line, "%s%d%d", str, &request_id, &payload) == 3)
			if (!strcmp(str, "$BLOCK"))
			{
				/* found a block */
				if (i < MAXHEAP) {
					heap[i].request_id = request_id;
					heap[i].payload = payload;
					i++;
				}
			}
	}

	fclose(fp);
	unlink(file);

	return i;
}


/*
 * Functions to print the expected output and the user input for heapcheck
 */

static void expected_blocks(trace_t* trace)
{
	int active = 0;
	int j;

	printf("[%s]期待从HeapCheck中的输出:\n", __func__);
	for (j = 0; j < trace->num_ids; j++)
		if (trace->allocated[j] == 1)
		{
			active++;
			printf("$BLOCK %d %zu\n", j, trace->block_sizes[j]);
		}
	printf("[%s]真正有效的块数 = %d\n", __func__, active);
}

static void outputted_blocks(block_t* heap, int heapsize)
{
	int i;
	printf("[%s]mm_heapcheck产生的输出\n", __func__);
	for (i = 0; i < heapsize; i++)
	{
		printf("[%s]$BLOCK %d %d\n", __func__, heap[i].request_id, heap[i].payload);
	}
	printf("[%s]输出有效的块数 = %d\n", __func__, heapsize);
}

/*
 * usage - Explain the command line arguments
 */
static void usage(char* exec)
{
	//	fprintf(stderr, "Usage: %s [-hvVal] [-f <file>] [-t <dir>]\n", exec);
	fprintf(stderr, "Usage: %s [-hvVl] [-f <file>] [-t <dir>]\n", exec);
	fprintf(stderr, "Options\n");
	//	fprintf(stderr, "\t-a         不检查团队信息.\n");
	fprintf(stderr, "\t-f <file>  使用<file>作为trace文件.\n");
	fprintf(stderr, "\t-g         为自动评分创建一个摘要信息.\n");
	fprintf(stderr, "\t-h         打印此信息.\n");
	fprintf(stderr, "\t-l         运行libc malloc的检测.\n");
	fprintf(stderr, "\t-t <dir>   缺省trace存放的目录.\n");
	fprintf(stderr, "\t-v         打印每条trace的性能.\n");
	fprintf(stderr, "\t-V         打印额外的调试信息.\n");
}





