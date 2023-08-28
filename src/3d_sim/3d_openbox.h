#ifndef _3D_OPENBOX_H_
#define _3D_OPENBOX_H_

#include "utils/my_rand.h"
#include "utils/octree.h"


// particle radius (in pixels)
#define RADIUS          0.5
#define BOX_HALF_DIM    4.827446923028148

// #define TIMESTEP        0.01
#define NUM_STEPS       10000
extern double TIMESTEP;


typedef struct {
    double width;
    double height;
    double depth;
    Octree *tree;    /* for faster collision detection */

    /* for faster dumping/outputting of particles */
    int num_particles;
    ListNode **particles;
} OpenBox;

typedef struct {
    double v_x; // x component of Velocity
    double v_y; // y component of Velocity
    double v_z;
    double v_squared;   // kinetic energy
    double PE;  // potential energy
    double a_x; // x component of acceleration
    double a_y; // y component of acceleration
    double a_z;
} Motion;


// OpenBox* new_openbox(const double width, const double height)

OpenBox* new_regular(Seed * const seed, const double width,
                        const double height, const double depth);
OpenBox* new_random(Seed * const seed, const double width,
                        const double height, const double depth);

int init_velocities(OpenBox * const box, Seed * const seed);
void* update_interact(void * const job_data); // multithreaded
void* update_velocity(void * const job_data); // multithreaded
int update_position(OpenBox * const box, double dt);

int delete_openbox(OpenBox * const box);


// helper functions

int find_repeats(OpenBox * const box, const double x, const double y,
    const double z, const double extent, double * const x_pos,
    double * const y_pos, double * const z_pos);

int does_collide(Octree * const tree, const double x, const double y,
    const double z);

// int output_OpenBox(OpenBox * const box, char * const name);
// int analyze_Velocity2D(OpenBox * const box, char * const name);


#endif

