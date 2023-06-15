/* Routines for using cycle counter */

/* Start the counter */
void start_counter(void);

/* Get # cycles since counter started */
double get_counter(void);

double mhz(int verbose);
void make_CPU_busy(void);