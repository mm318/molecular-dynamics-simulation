#ifndef _A3P1_MAIN_H_
#define _A3P1_MAIN_H_


#include "3d_openbox.h"


typedef struct {
    Seed seed;
    OpenBox *box;
    double dt;          // timestep
    int first_index;    // index of first particle
    int last_index;     // index of last particle (excluded)
} Job;

// for(i=first_index;i<last_index;i++)

#endif

