#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "2d_openbox.h"


static OpenBox2D* new_2d_openbox(const double width, const double height)
{
    // double half_height = height / 2.0;
    // double half_width = width / 2.0;
    OpenBox2D *box = (OpenBox2D*) malloc(sizeof(OpenBox2D));

    // initialize box
    box->height = height;
    box->width = width;
    // box->tree = new_quadtree(-half_height, half_height, -half_width, half_width);
    box->tree = new_quadtree(0, height, 0, width);
    box->num_particles = 0;
    box->particles = NULL;

    return box;
}


// prevent creating particles that overlap
const double min_d_squared = 4.0*RADIUS*RADIUS;
int does_collide(QTree * const tree, const double y, const double x)
{
    int i;

    // make particle bigger than it actually is, so more robust detection
    const double x_min = x - 2*RADIUS;
    const double x_max = x + 2*RADIUS;
    const double y_min = y - 2*RADIUS;
    const double y_max = y + 2*RADIUS;

    if(y_min > tree->y_max || y_max < tree->y_min || x_min > tree->x_max ||
       x_max < tree->x_min) {
        return 0;
    }

    if(tree->num_points <= 0) {
        if (y_min <= tree->y_mid && x_min <= tree->x_mid &&
            tree->quadrants[LEFT_UP] != NULL &&
            does_collide(tree->quadrants[LEFT_UP], y, x)) {
            return 1;
        }

        if (y_min <= tree->y_mid && x_max >= tree->x_mid &&
            tree->quadrants[RIGHT_UP] != NULL &&
            does_collide(tree->quadrants[RIGHT_UP], y, x)) {
            return 1;
        }

        if (y_max >= tree->y_mid && x_min <= tree->x_mid &&
            tree->quadrants[LEFT_DOWN] != NULL &&
            does_collide(tree->quadrants[LEFT_DOWN], y, x)) {
            return 1;
        }

        if (y_max >= tree->y_mid && x_max >= tree->x_mid &&
            tree->quadrants[RIGHT_DOWN] != NULL &&
            does_collide(tree->quadrants[RIGHT_DOWN], y, x)) {
            return 1;
        }
    } else {
        double dy, dx;
        ListNode *cn;   /* current node pointer */

        cn = tree->head;
        for(i=0; i<tree->num_points; i++) {
            dy = cn->y_pos - y;
            dx = cn->x_pos - x;
            if((dy*dy + dx*dx) < min_d_squared) {
                return 1;
            }
            cn = cn->next;
        }
    }

    return 0;
}

int find_repeats(OpenBox2D * const box, double x, double y, double extent,
                 double * const x_pos, double * const y_pos)
{
    int num_repeat = 0; // how many repeats due to periodicity (1 to 4)
    double y_min, y_max, x_min, x_max;  // extent of the particle

    // original position
    x_pos[0] = x;
    y_pos[0] = y;
    // make particle bigger than it actually is, so more robust detection
    x_min = x - extent;
    x_max = x + extent;
    y_min = y - extent;
    y_max = y + extent;
    num_repeat = 1;

    // repeat in the vertical direction
    if(y_min < 0 || y_max > box->height) {
        if(y_min < 0) {
            y_pos[num_repeat] = y_pos[0] + box->height;
        } else {
            y_pos[num_repeat] = y_pos[0] - box->height;
        }
        x_pos[num_repeat] = x_pos[0];
        num_repeat++;
    }

    // repeat in the horizontal direction
    if(x_min < 0 || x_max > box->width) {
        if(x_min < 0) {
            x_pos[num_repeat] = x_pos[0] + box->width;
        } else {
            x_pos[num_repeat] = x_pos[0] - box->width;
        }
        y_pos[num_repeat] = y_pos[0];
        num_repeat++;
    }

    // repeat in the diagonal direction
    // only occurs when both the vertical and horizontal repeat
    if(num_repeat == 3) {
        y_pos[3] = y_pos[1];
        x_pos[3] = x_pos[2];
        num_repeat++;
    }

    return num_repeat;
}


// for approximately 360 particles
#define NUM_PARTICLES   360
#define NUM_PER_ROW 19
#define NUM_ROWS    19


OpenBox2D* new_regular(Seed * const seed, const double width, const double height)
{
    int i, j, n;
    OpenBox2D *box;
    double x, y, x_step, y_step;
    ListNode *cn;

    box = new_2d_openbox(height, width);
    box->num_particles = NUM_PER_ROW*NUM_ROWS;
    box->particles = (ListNode**) malloc(box->num_particles*sizeof(ListNode*));

    // fprintf(stderr,"generating particles\n");

    x_step = width / NUM_PER_ROW;
    y_step = height / NUM_ROWS;
    x = box->width*myrand_double(seed);
    y = box->height*myrand_double(seed);
    n = 0;
    for(i=0; i<NUM_ROWS; i++) {
        for(j=0; j<NUM_PER_ROW; j++) {
            // fprintf(stderr,"(row: %d, num: %d) generating particle %d\n", i, j, n);

            cn = quadtree_add(box->tree, y, x, NULL);
            box->particles[n] = cn;

            x += x_step;
            if(x > width) {
                x -= width;
            }
            n++;
        }
        y += y_step;
        if(y > height) {
            y -= height;
        }
    }

    return box;
}

OpenBox2D* new_random(Seed * const seed, const double width, const double height)
{
    int i, j;
    OpenBox2D *box;
    double y[4], x[4];
    int num_repeat = 0; // how many repeats due to periodicity (1 to 4)
    int collides;
    ListNode *new_particle;

    box = new_2d_openbox(height, width);
    box->num_particles = NUM_PARTICLES;
    box->particles = (ListNode**) malloc(NUM_PARTICLES*sizeof(ListNode*));

    for(i=0; i<NUM_PARTICLES; i++) {
        y[0] = box->height*myrand_double(seed);
        x[0] = box->width*myrand_double(seed);
        num_repeat = find_repeats(box, x[0], y[0], 2*RADIUS, x, y);

        collides = 0;
        for(j=0; j<num_repeat; j++) {
            if(does_collide(box->tree, y[j], x[j])) {
                collides = 1;
                break;
            }
        }

        if(collides) {
            i--;
        } else {
            new_particle = quadtree_add(box->tree, y[0], x[0], NULL);
            box->particles[i] = new_particle;
        }
    }

    return box;
}


int delete_2d_openbox(OpenBox2D * const box)
{
    delete_quadtree(box->tree);
    free(box->particles);
    free(box);
    return 0;
}

