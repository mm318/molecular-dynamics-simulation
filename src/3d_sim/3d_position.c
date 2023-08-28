#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "3d_openbox.h"


static OpenBox* new_openbox(const double width, const double height,
                            const double depth)
{
    // double half_height = height / 2.0;
    // double half_width = width / 2.0;
    OpenBox *box = (OpenBox*) malloc(sizeof(OpenBox));

    // initialize box
    box->width = width;
    box->height = height;
    box->depth = depth;
    box->tree = new_octree(0, height, 0, width, 0, depth);
    box->num_particles = 0;
    box->particles = NULL;

    return box;
}


// prevent creating particles that overlap
const double min_d_squared = 4.0*RADIUS*RADIUS;
int does_collide(Octree * const tree, const double x, const double y,
                 const double z)
{
    int i;

    // make particle bigger than it actually is, so more robust detection
    const double x_min = x - 2*RADIUS;
    const double x_max = x + 2*RADIUS;
    const double y_min = y - 2*RADIUS;
    const double y_max = y + 2*RADIUS;
    const double z_min = z - 2*RADIUS;
    const double z_max = z + 2*RADIUS;

    if(x_min > tree->max.x || x_max < tree->min.x ||
       y_min > tree->max.y || y_max < tree->min.y ||
       z_min > tree->max.z || z_max < tree->min.z) {
        return 0;
    }

    if(tree->num_points <= 0) {
        if(x_min <= tree->mid.x && y_max >= tree->mid.y && z_max >= tree->mid.z &&
           tree->octants[LEFT_UP_NEAR] != NULL &&
           does_collide(tree->octants[LEFT_UP_NEAR], x, y, z)) {
            return 1;
        }

        if(x_max >= tree->mid.x && y_max >= tree->mid.y && z_max >= tree->mid.z &&
           tree->octants[RIGHT_UP_NEAR] != NULL &&
           does_collide(tree->octants[RIGHT_UP_NEAR], x, y, z)) {
            return 1;
        }

        if(x_min <= tree->mid.x && y_min <= tree->mid.y && z_max >= tree->mid.z &&
           tree->octants[LEFT_DOWN_NEAR] != NULL &&
           does_collide(tree->octants[LEFT_DOWN_NEAR], x, y, z)) {
            return 1;
        }

        if(x_max >= tree->mid.x && y_min <= tree->mid.y && z_max >= tree->mid.z &&
           tree->octants[RIGHT_DOWN_NEAR] != NULL &&
           does_collide(tree->octants[RIGHT_DOWN_NEAR], x, y, z)) {
            return 1;
        }

        if(x_min <= tree->mid.x && y_max >= tree->mid.y && z_min <= tree->mid.z &&
           tree->octants[LEFT_UP_FAR] != NULL &&
           does_collide(tree->octants[LEFT_UP_FAR], x, y, z)) {
            return 1;
        }

        if(x_max >= tree->mid.x && y_max >= tree->mid.y && z_min <= tree->mid.z &&
           tree->octants[RIGHT_UP_FAR] != NULL &&
           does_collide(tree->octants[RIGHT_UP_FAR], x, y, z)) {
            return 1;
        }

        if(x_min <= tree->mid.x && y_min <= tree->mid.y && z_min <= tree->mid.z &&
           tree->octants[LEFT_DOWN_FAR] != NULL &&
           does_collide(tree->octants[LEFT_DOWN_FAR], x, y, z)) {
            return 1;
        }

        if(x_max >= tree->mid.x && y_min <= tree->mid.y && z_min <= tree->mid.z &&
           tree->octants[RIGHT_DOWN_FAR] != NULL &&
           does_collide(tree->octants[RIGHT_DOWN_FAR], x, y, z)) {
            return 1;
        }
    } else {
        double dy, dx, dz;
        ListNode *cn;   /* current node pointer */

        cn = tree->head;
        for(i=0; i<tree->num_points; i++) {
            dx = cn->pos.x - x;
            dy = cn->pos.y - y;
            dz = cn->pos.z - z;
            if((dy*dy + dx*dx + dz*dz) < min_d_squared) {
                return 1;
            }
            cn = cn->next;
        }
    }

    return 0;
}

// x_pos, y_pos, z_pos need to be able to store eight elements
int find_repeats(OpenBox * const box, const double x, const double y,
                 const double z, const double extent, double * const x_pos,
                 double * const y_pos, double * const z_pos)
{
    int num_repeat = 0; // how many repeats due to periodicity (1 to 4)
    char x_repeat, y_repeat, z_repeat;
    double new_x, new_y, new_z;
    double x_min, x_max, y_min, y_max, z_min, z_max;    // extent of the particle

    // original position
    x_pos[0] = x;
    y_pos[0] = y;
    z_pos[0] = z;
    num_repeat = 1; // original coordinates

    // make particle bigger than it actually is, so more robust detection
    x_min = x - extent;
    x_max = x + extent;
    y_min = y - extent;
    y_max = y + extent;
    z_min = z - extent;
    z_max = z + extent;

    x_repeat = 0;
    y_repeat = 0;
    z_repeat = 0;

    // repeat in the x direction
    if(x_min < 0 || x_max > box->width) {
        if(x_min < 0) {
            new_x = x_pos[0] + box->width;
        } else {
            new_x = x_pos[0] - box->width;
        }
        x_pos[num_repeat] = new_x;
        y_pos[num_repeat] = y_pos[0];
        z_pos[num_repeat] = z_pos[0];
        num_repeat++;
        x_repeat = 1;
    }

    // repeat in the y direction
    if(y_min < 0 || y_max > box->height) {
        if(y_min < 0) {
            new_y = y_pos[0] + box->height;
        } else {
            new_y = y_pos[0] - box->height;
        }
        x_pos[num_repeat] = x_pos[0];
        y_pos[num_repeat] = new_y;
        z_pos[num_repeat] = z_pos[0];
        num_repeat++;
        y_repeat = 1;
    }

    // repeat in the z direction
    if(z_min < 0 || z_max > box->depth) {
        if(z_min < 0) {
            new_z = z_pos[0] + box->depth;
        } else {
            new_z = z_pos[0] - box->depth;
        }
        x_pos[num_repeat] = x_pos[0];
        y_pos[num_repeat] = y_pos[0];
        z_pos[num_repeat] = new_z;
        num_repeat++;
        z_repeat = 1;
    }

    // repeat in the xy diagonal direction
    if(x_repeat && y_repeat) {
        x_pos[num_repeat] = new_x;
        y_pos[num_repeat] = new_y;
        z_pos[num_repeat] = z_pos[0];
        num_repeat++;
    }

    // repeat in the xz diagonal direction
    if(x_repeat && z_repeat) {
        x_pos[num_repeat] = new_x;
        y_pos[num_repeat] = y_pos[0];
        z_pos[num_repeat] = new_z;
        num_repeat++;
    }

    // repeat in the yz diagonal direction
    if(y_repeat && z_repeat) {
        x_pos[num_repeat] = x_pos[0];
        y_pos[num_repeat] = new_y;
        z_pos[num_repeat] = new_z;
        num_repeat++;
    }

    // repeat in the xyz diagonal direction
    if(x_repeat && y_repeat && z_repeat) {
        x_pos[num_repeat] = new_x;
        y_pos[num_repeat] = new_y;
        z_pos[num_repeat] = new_z;
        num_repeat++;
    }

    return num_repeat;
}


// for approximately 360 particles
#define NUM_PARTICLES   360
#define NUM_PER_ROW 19
#define NUM_ROWS    19

OpenBox* new_regular(Seed * const seed, const double width, const double height,
                     const double depth)
{
    return NULL;
}

OpenBox* new_random(Seed * const seed, const double width, const double height,
                    const double depth)
{
    int i, j;
    OpenBox *box;
    double x[8], y[8], z[8];
    int num_repeat = 0; // how many repeats due to periodicity (1 to 4)
    int collides;
    ListNode *new_particle;

    box = new_openbox(height, width, depth);
    box->num_particles = NUM_PARTICLES;
    box->particles = (ListNode**) malloc(NUM_PARTICLES*sizeof(ListNode*));

    for(i=0; i<NUM_PARTICLES; i++) {
        x[0] = box->width*myrand_double(seed);
        y[0] = box->height*myrand_double(seed);
        z[0] = box->depth*myrand_double(seed);

        num_repeat = find_repeats(box, x[0], y[0], z[0], 2*RADIUS, x, y, z);

        collides = 0;
        for(j=0; j<num_repeat; j++) {
            if(does_collide(box->tree, x[j], y[j], z[j])) {
                collides = 1;
                break;
            }
        }

        if(collides) {
            i--;
        } else {
            new_particle = octree_add(box->tree, x[0], y[0], z[0], NULL);
            box->particles[i] = new_particle;
        }
    }

    return box;
}


int delete_openbox(OpenBox * const box)
{
    delete_octree(box->tree);
    free(box->particles);
    free(box);
    return 0;
}
