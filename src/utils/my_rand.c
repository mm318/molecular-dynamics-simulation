#include <math.h>
#include "my_rand.h"


const uint64_t a = 1103515245;
const uint64_t c = 12345;
const uint64_t m = 4294967296;
const double m_dbl = (double) 4294967296;


static uint64_t lcg(uint64_t * const x0)
{
    uint64_t x1;
    x1 = (a*(*x0) + c) % m;
    *x0 = x1;
    return x1;
}

// uniformly distributed random integer
int myrand_int(Seed * const seed)
{
    return (int) lcg(seed);
}

// uniformly distributed random real
double myrand_double(Seed * const seed)
{
    double res;
    res = ((double) lcg(seed))/((double) m_dbl);
    return res;
}

// using box muller method, standard normal distribution
int myrand_norm(Seed * const seed, double * const n1, double * const n2)
{
    Seed s1, s2;
    double u1, u2;

    s1 = (Seed) lcg(seed);
    s2 = (Seed) lcg(seed);
    u1 = myrand_double(&s1);
    u2 = myrand_double(&s2);

    /* x = stddev*sqrt(-2*ln(u1))*cos(2*pi*u2) */
    /* y = stddev*sqrt(-2*ln(u1))*sin(2*pi*u2) */
    *n1 = sqrt(-2*log(u1))*cos(2*M_PI*u2);
    *n2 = sqrt(-2*log(u1))*sin(2*M_PI*u2);

    return 0;
}

