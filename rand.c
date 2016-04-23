#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "rand.h"

static __inline__ unsigned long long tick();
static __inline__ unsigned tick_lo();

int
get_irand(int from, int to) {
	srand (time(NULL) ^ (tick_lo()<<5));
	return (rand()%(to-from+1)) + from;
}

long long
get_llrand(long long from, long long to) {
	srand (time(NULL) ^ (tick_lo()<<5));
	return ((((long long)rand())<<32) | ((long long)rand()))
			% (to-from+1) + from;
}

/*
 * Returns nonnegative double-precision floating-point values 
 * uniformly distributed between [0.0, 1.0)
 */
double
get_drand(){
	srand48 (tick() ^ (time(NULL)<<5));
	return drand48();
}

static __inline__ unsigned long long
tick() {
  unsigned hi, lo;
  __asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
  return ( (unsigned long long)lo)|( ((unsigned long long)hi)<<32 );
}

static __inline__ unsigned
tick_lo() {
  unsigned hi, lo;
  __asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
  return lo;
}
