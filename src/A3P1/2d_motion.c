#include <stdio.h>
#include <stdlib.h>
// #include <inttypes.h>
#include <math.h>

#include "A3P1_main.h"


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

int init_velocities(OpenBox2D * const box, Seed * const seed)
{
    int i;
    double vy, vx;
    double vy_sum, vx_sum, delta_vy, delta_vx;

    Motion2D *v = (Motion2D*) malloc(box->num_particles*sizeof(Motion2D));

    vy_sum = 0;
    vx_sum = 0;
    for(i=0; i<box->num_particles; i++) {
        myrand_norm(seed, &vx, &vy);
        v[i].v_y = vy;
        v[i].v_x = vx;
        vy_sum += vy;
        vx_sum += vx;
    }

    // removing center of mass momentum
    delta_vy = -vy_sum/((double) box->num_particles);
    delta_vx = -vx_sum/((double) box->num_particles);
    for(i=0; i<box->num_particles; i++) {
        v[i].v_y += delta_vy;
        v[i].v_x += delta_vx;
        box->particles[i]->data = (void*) &v[i];
    }

    return 0;
}


static void calc_interact(QTree * const tree, ListNode * const p1,
                          const double x, const double y)
{
    // make particle bigger than it actually is, so more robust detection
    const double y_min = y - CUTOFF;
    const double y_max = y + CUTOFF;
    const double x_min = x - CUTOFF;
    const double x_max = x + CUTOFF;

    if(y_min > tree->y_max || y_max < tree->y_min || x_min > tree->x_max ||
       x_max < tree->x_min) {
        return;
    }

    if(tree->num_points <= 0) {
        if (y_min <= tree->y_mid && x_min <= tree->x_mid &&
            tree->upper_left != NULL) {
            calc_interact(tree->upper_left, p1, x, y);
        }

        if (y_min <= tree->y_mid && x_max >= tree->x_mid &&
            tree->upper_right != NULL) {
            calc_interact(tree->upper_right, p1, x, y);
        }

        if (y_max >= tree->y_mid && x_min <= tree->x_mid &&
            tree->lower_left != NULL) {
            calc_interact(tree->lower_left, p1, x, y);
        }

        if (y_max >= tree->y_mid && x_max >= tree->x_mid &&
            tree->lower_right != NULL) {
            calc_interact(tree->lower_right, p1, x, y);
        }
    } else {
        int i;
        double dy, dx, r, F;
        Motion2D *v = (Motion2D*) p1->data;
        ListNode *p2;   /* pointer to current node being compared */

        p2 = tree->head;
        for(i=0; i<tree->num_points; i++) {
            if(p1 != p2) {
                dy = p2->y_pos - y;
                dx = p2->x_pos - x;
                r = sqrt(dy*dy + dx*dx);

                if(r <= CUTOFF) {
                    if(r == 0) {
                        fprintf(stderr, "[Warning] zero separation distance!\n");
                    } else {
                        v->PE += lennard_jones_potential(r);

                        F = lennard_jones_force(r);
                        v->a_x += dx/r*F;
                        v->a_y += dy/r*F;
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
    Job2D *job = (Job2D*) job_data;
    OpenBox2D *box = job->box;
    ListNode *cn;
    Motion2D *v;
    int num_repeat;
    double x_pos[4], y_pos[4];

    for(i=job->first_index; i<job->last_index; i++) {
        cn = box->particles[i];

        v = (Motion2D*) cn->data;
        v->v_squared = v->v_x*v->v_x + v->v_y*v->v_y;
        v->PE = 0;  // init
        v->a_x = 0; // init
        v->a_y = 0; // init

        num_repeat = find_repeats(box, cn->x_pos, cn->y_pos, CUTOFF, x_pos, y_pos);
        for(j=0; j<num_repeat; j++) {
            calc_interact(box->tree, cn, x_pos[j], y_pos[j]);
        }
    }

    return NULL;
}

void* update_velocity(void * const job_data)
{
    int i;
    Job2D *job = (Job2D*) job_data;
    OpenBox2D *box = job->box;
    ListNode *cn;
    Motion2D *v;

    for(i=job->first_index; i<job->last_index; i++) {
        cn = box->particles[i];

        v = (Motion2D*) cn->data;
        v->v_x += 0.5*job->dt*v->a_x;
        v->v_y += 0.5*job->dt*v->a_y;
    }

    return NULL;
}

int update_position(OpenBox2D * const box, double dt)
{
    int i;
    ListNode *cn, *new_node;
    Motion2D *v;
    double new_x, new_y;

    QTree *new_tree = new_quadtree(0, box->height, 0, box->width);
    ListNode **new_particles = (ListNode**)
                               malloc(box->num_particles*sizeof(ListNode*));

    for(i=0; i<box->num_particles; i++) {
        cn = box->particles[i];
        v = (Motion2D*) cn->data;

        new_x = cn->x_pos + dt*v->v_x;
        new_x = fmod(new_x, box->width);
        while(new_x < 0) {
            new_x += box->width;
        }
        while(new_x > box->width) {
            new_x -= box->width;
        }
        // multiple_x = floor(new_x / box->width);
        // new_x -= multiple_x*box->width;

        new_y = cn->y_pos + dt*v->v_y;
        new_y = fmod(new_y, box->height);
        while(new_y < 0) {
            new_y += box->height;
        }
        while(new_y > box->width) {
            new_y -= box->height;
        }
        // multiple_y = floor(new_y / box->height);
        // new_y -= multiple_y*box->height;

        // debug
        // if(new_x < 0 || new_x > box->width || new_y < 0 || new_y > box->height) {
        // fprintf(stderr, "[Warning] out of bounds detected (%f, %f) (%f, %f)\n",
        // new_x, new_y, multiple_x, multiple_y);
        // }

        new_node = quadtree_add(new_tree, new_y, new_x, (void*) v);
        new_particles[i] = new_node;
    }

    delete_quadtree(box->tree);
    free(box->particles);
    box->tree = new_tree;
    box->particles = new_particles;

    return 0;
}
