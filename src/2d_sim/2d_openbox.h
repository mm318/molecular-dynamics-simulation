#ifndef _2D_OPENBOX_H_
#define _2D_OPENBOX_H_

#include "utils/my_rand.h"
#include "utils/quadtree.h"


// particle radius (in pixels)
#define RADIUS          0.5
#define BOX_HALF_DIM    15

// #define TIMESTEP        0.01
#define NUM_STEPS       10000
extern double TIMESTEP;


typedef struct {
    double width;
    double height;
    QTree *tree;    /* for faster collision detection */

    /* for faster dumping/outputting of particles */
    int num_particles;
    ListNode **particles;
} OpenBox2D;

typedef struct {
    double v_x; // x component of Velocity
    double v_y; // y component of Velocity
    double v_squared;   // kinetic energy
    double PE;  // potential energy
    double a_x; // x component of acceleration
    double a_y; // y component of acceleration
} Motion2D;


// OpenBox2D* new_openbox(const double width, const double height)

OpenBox2D* new_regular(Seed * const seed, const double width,
                        const double height);
OpenBox2D* new_random(Seed * const seed, const double width,
                        const double height);

int init_velocities(OpenBox2D * const box, Seed * const seed);
void* update_interact(void * const job_data); // multithreaded
void* update_velocity(void * const job_data); // multithreaded
int update_position(OpenBox2D * const box, double dt);

int delete_2d_openbox(OpenBox2D * const box);


// helper functions

int find_repeats(OpenBox2D * const box, double x, double y, double extent,
                 double * const x_pos, double * const y_pos);

int does_collide(QTree * const tree, const double y, const double x);

// int output_OpenBox2D(OpenBox2D * const box, char * const name);
// int analyze_Velocity2D(OpenBox2D * const box, char * const name);


#endif

