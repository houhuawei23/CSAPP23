/* Routines for using cycle counter */

/* Start the counter */
void start_counter(void);

/* Get # cycles since counter started.  Returns 1e20 if detect timing anomaly */
double get_counter(void);
void make_CPU_busy(void);

double mhz(int verbose);
double CPU_Factor(void);
//double GetCpuClock(void);