#ifndef _MYRAND_H_
#define _MYRAND_H_

#include <inttypes.h>

#define M_PI    3.14159265358979323846


typedef uint64_t Seed;

// uint64_t lcg(uint64_t* const restrict seed);
int myrand_int(Seed * const seed);
double myrand_double(Seed * const seed);
int myrand_norm(Seed * const seed, double * const n1, double * const n2);


#endif
