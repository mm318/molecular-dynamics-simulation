#include <stdio.h>
#include <stdlib.h>
// #include <inttypes.h>
#include <math.h>

#include "3d_sim_main.h"


// init position            (new_random)
// init velocity            (init_velocities)
// force                    (update_interact)

// velocity (t + 1/2 dt)    (update_velocity)
// position (t + dt)        (update_position)
// force    (t + dt)        (update_interact)
// velocity (t + dt)        (update_velocity)

// velocity (t + dt + 1/2 dt)   (update_velocity)
// ...


// CUTOFF = 2.5*2*RADIUS
#define CUTOFF  2.5
double TIMESTEP;


static const double _48_c13 = 48.0*pow(CUTOFF, -13);
static const double _52_c12 = 52.0*pow(CUTOFF, -12);
static const double _24_c7 = 24.0*pow(CUTOFF, -7);
static const double _28_c6 = 28.0*pow(CUTOFF, -6);

// 48/c^13 - 24/c^7 - 48/r^13 + 24/r^7
double lennard_jones_force(double r)
{
    return (_48_c13 - _24_c7 - 48.0*pow(r, -13) + 24.0*pow(r,-7));
}

// 4/r^12 - 4/r^6 + (48 r)/c^13 - (24 r)/c^7 -52/c^12 + 28/c^6
double lennard_jones_potential(double r)
{
    return 4*(pow(r,-12) - pow(r,-6)) + r*(_48_c13 - _24_c7) - _52_c12 + _28_c6;
}

int init_velocities(OpenBox * const box, Seed * const seed)
{
    int i;
    double vx, vy, vz, v_dummy;
    double vx_sum, vy_sum, vz_sum, dvx, dvy, dvz;

    Motion *v = (Motion*) malloc(box->num_particles*sizeof(Motion));

    vx_sum = 0;
    vy_sum = 0;
    vz_sum = 0;
    for(i=0; i<box->num_particles; i++) {
        myrand_norm(seed, &vx, &vy);
        myrand_norm(seed, &vz, &v_dummy);
        v[i].v_x = vx;
        v[i].v_y = vy;
        v[i].v_z = vz;
        vx_sum += vx;
        vy_sum += vy;
        vz_sum += vz;
    }

    // removing center of mass momentum
    dvx = -vx_sum/((double) box->num_particles);
    dvy = -vy_sum/((double) box->num_particles);
    dvz = -vz_sum/((double) box->num_particles);
    for(i=0; i<box->num_particles; i++) {
        v[i].v_x += dvx;
        v[i].v_y += dvy;
        v[i].v_z += dvz;
        // fprintf(stderr,"velocities #%d: (%f, %f, %f)\n", i,
            // v[i].v_x, v[i].v_y, v[i].v_z);
        box->particles[i]->data = (void*) &v[i];
    }

    return 0;
}

static void calc_interact(Octree * const tree, ListNode * const p1,
                          const double x, const double y, const double z)
{
    // make particle bigger than it actually is, so more robust detection
    const double x_min = x - CUTOFF;
    const double x_max = x + CUTOFF;
    const double y_min = y - CUTOFF;
    const double y_max = y + CUTOFF;
    const double z_min = z - CUTOFF;
    const double z_max = z + CUTOFF;

    if(x_min > tree->max.x || x_max < tree->min.x ||
       y_min > tree->max.y || y_max < tree->min.y ||
       z_min > tree->max.z || z_max < tree->min.z) {
        return;
    }

    if(tree->num_points <= 0) {
        if(x_min <= tree->mid.x && y_max >= tree->mid.y && z_max >= tree->mid.z &&
           tree->octants[LEFT_UP_NEAR] != NULL) {
            calc_interact(tree->octants[LEFT_UP_NEAR], p1, x, y, z);
        }

        if(x_max >= tree->mid.x && y_max >= tree->mid.y && z_max >= tree->mid.z &&
           tree->octants[RIGHT_UP_NEAR] != NULL) {
            calc_interact(tree->octants[RIGHT_UP_NEAR], p1, x, y, z);
        }

        if(x_min <= tree->mid.x && y_min <= tree->mid.y && z_max >= tree->mid.z &&
           tree->octants[LEFT_DOWN_NEAR] != NULL) {
            calc_interact(tree->octants[LEFT_DOWN_NEAR], p1, x, y, z);
        }

        if(x_max >= tree->mid.x && y_min <= tree->mid.y && z_max >= tree->mid.z &&
           tree->octants[RIGHT_DOWN_NEAR] != NULL) {
            calc_interact(tree->octants[RIGHT_DOWN_NEAR], p1, x, y, z);
        }

        if(x_min <= tree->mid.x && y_max >= tree->mid.y && z_min <= tree->mid.z &&
           tree->octants[LEFT_UP_FAR] != NULL) {
            calc_interact(tree->octants[LEFT_UP_FAR], p1, x, y, z);
        }

        if(x_max >= tree->mid.x && y_max >= tree->mid.y && z_min <= tree->mid.z &&
           tree->octants[RIGHT_UP_FAR] != NULL) {
            calc_interact(tree->octants[RIGHT_UP_FAR], p1, x, y, z);
        }

        if(x_min <= tree->mid.x && y_min <= tree->mid.y && z_min <= tree->mid.z &&
           tree->octants[LEFT_DOWN_FAR] != NULL) {
            calc_interact(tree->octants[LEFT_DOWN_FAR], p1, x, y, z);
        }

        if(x_max >= tree->mid.x && y_min <= tree->mid.y && z_min <= tree->mid.z &&
           tree->octants[RIGHT_DOWN_FAR] != NULL) {
            calc_interact(tree->octants[RIGHT_DOWN_FAR], p1, x, y, z);
        }
    } else {
        int i;
        double dy, dx, dz, r, F;
        Motion *v = (Motion*) p1->data;
        ListNode *p2;   /* pointer to current node being compared */

        p2 = tree->head;
        for(i=0; i<tree->num_points; i++) {
            if(p1 != p2) {
                dx = p2->pos.x - x;
                dy = p2->pos.y - y;
                dz = p2->pos.z - z;
                r = sqrt(dy*dy + dx*dx + dz*dz);

                if(r <= CUTOFF) {
                    if(r == 0) {
                        fprintf(stderr, "[Warning] zero separation distance!\n");
                    } else {
                        v->PE += lennard_jones_potential(r);

                        F = lennard_jones_force(r);
                        v->a_x += dx/r*F;
                        v->a_y += dy/r*F;
                        v->a_z += dz/r*F;
                    }
                }
            }
            p2 = p2->next;
        }
    }

    return;
}

void* update_interact(void * const job_data)
{
    int i, j;
    Job *job = (Job*) job_data;
    OpenBox *box = job->box;
    ListNode *cn;
    Motion *v;
    int num_repeat;
    double x_pos[8], y_pos[8], z_pos[8];

    for(i=job->first_index; i<job->last_index; i++) {
        cn = box->particles[i];
        v = (Motion*) cn->data;

        v->v_squared = v->v_x*v->v_x + v->v_y*v->v_y + v->v_z*v->v_z;
        v->PE = 0;  // init
        v->a_x = 0; // init
        v->a_y = 0; // init
        v->a_z = 0; // init

        num_repeat = find_repeats(box, cn->pos.x, cn->pos.y, cn->pos.z,
                                  CUTOFF, x_pos, y_pos, z_pos);

        for(j=0; j<num_repeat; j++) {
            calc_interact(box->tree, cn, x_pos[j], y_pos[j], z_pos[j]);
        }
    }

    return NULL;
}

void* update_velocity(void * const job_data)
{
    int i;
    Job *job = (Job*) job_data;
    OpenBox *box = job->box;
    ListNode *cn;
    Motion *v;

    for(i=job->first_index; i<job->last_index; i++) {
        cn = box->particles[i];
        v = (Motion*) cn->data;
        v->v_x += 0.5*job->dt*v->a_x;
        v->v_y += 0.5*job->dt*v->a_y;
        v->v_z += 0.5*job->dt*v->a_z;
    }

    return NULL;
}

int update_position(OpenBox * const box, double dt)
{
    int i;
    ListNode *cn, *new_node;
    Motion *v;
    double new_x, new_y, new_z;

    Octree *new_tree = new_octree(0, box->height, 0, box->width, 0, box->depth);
    ListNode **new_particles = (ListNode**)
                               malloc(box->num_particles*sizeof(ListNode*));

    for(i=0; i<box->num_particles; i++) {
        cn = box->particles[i];
        v = (Motion*) cn->data;

        new_x = cn->pos.x + dt*v->v_x;
        new_x = fmod(new_x, box->width);
        while(new_x < 0) {
            new_x += box->width;
        }
        while(new_x > box->width) {
            new_x -= box->width;
        }

        new_y = cn->pos.y + dt*v->v_y;
        new_y = fmod(new_y, box->height);
        while(new_y < 0) {
            new_y += box->height;
        }
        while(new_y > box->width) {
            new_y -= box->height;
        }

        new_z = cn->pos.z + dt*v->v_z;
        new_z = fmod(new_z, box->depth);
        while(new_z < 0) {
            new_z += box->depth;
        }
        while(new_z > box->depth) {
            new_z -= box->depth;
        }

        // debug
        // if(new_x < 0 || new_x > box->width || new_y < 0 || new_y > box->height) {
        // fprintf(stderr, "[Warning] out of bounds detected (%f, %f) (%f, %f)\n",
        // new_x, new_y, multiple_x, multiple_y);
        // }
        // fprintf(stderr, "adding particle %d\n", i);

        new_node = octree_add(new_tree, new_x, new_y, new_z, (void*) v);
        new_particles[i] = new_node;
    }

    delete_octree(box->tree);
    free(box->particles);
    box->tree = new_tree;
    box->particles = new_particles;

    return 0;
}

