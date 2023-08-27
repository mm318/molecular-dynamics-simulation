#include <stdio.h>
#include <time.h>
#include <math.h>
#include <pthread.h>
#include <sys/time.h>
#include <unistd.h>

#include "../lib/thr_pool/thr_pool.h"
#include "A3P1_main.h"
#include "2d_visual.h"


void analyze(OpenBox2D * const box, char * const run_name, int step, FILE *out)
{
    int i;
    double TE, Temp, vT_x, vT_y, vT;
    double v_squared_sum;
    ListNode *cn;
    Motion2D *v;

    if(step == 0) {
        fprintf(out, "%s Analysis:\n\n", run_name);
        fputs("t\tT.E.\tTemp.\tvT_x\tvT_y\tvT\n",out);
    }

    TE = 0;
    vT_x = 0;
    vT_y = 0;
    v_squared_sum = 0;
    for(i=0; i<box->num_particles; i++) {
        cn = box->particles[i];
        v = (Motion2D*) cn->data;

        v_squared_sum += v->v_squared;
        TE += v->PE;
        vT_x += v->v_x;
        vT_y += v->v_y;
    }

    TE /= 2;
    TE += v_squared_sum/2;
    // Temperature = v^2/(dof*num_particles)
    Temp = v_squared_sum/(2*box->num_particles);
    vT = sqrt(vT_x*vT_x + vT_y*vT_y);

    fprintf(out, "%f\t%f\t%f\t%.6g\t%.6g\t%.6g\n", step*TIMESTEP, TE, Temp,
            vT_x, vT_y, vT);

    // better for debug
    fflush(out);
}


typedef struct timeval TimeValue;

long time_elapsed(TimeValue *start, TimeValue *end)
{
    long utime, seconds, useconds;

    seconds = end->tv_sec - start->tv_sec;
    useconds = end->tv_usec - start->tv_usec;
    utime = 1000000*seconds + useconds;

    return utime;
}


#define NUM_THREADS 4

// void* bunched_update(void * const job_data)
// {
// update_interact(job_data);
// update_velocity(job_data);
// return NULL;
// }

int start_md(const int num_steps, const char output_png, char * const run_name)
{
    int i, k;
    char filename[1024];
    Seed s;

    VisualContext2D *vc;    // data for opengl display
    pthread_t vis_thr;      // thread for opengl visual

    int per_thread, remaining, curr_index;
    Job2D jobs[NUM_THREADS];
    thr_pool_t *workers;
    TimeValue start_run, end_run, start_force, end_force;
    long elapsed_run, elapsed_force;
    FILE *out = NULL;

    s = (Seed) time(NULL);
    // OpenBox2D *box = new_regular(&s, 2*BOX_HALF_DIM, 2*BOX_HALF_DIM);
    OpenBox2D *box = new_random(&s, 2*BOX_HALF_DIM, 2*BOX_HALF_DIM);

    fprintf(stderr,"creating new visual\n");
    vc = new_2d_visual(box);
    set_2d_visual(vc);
    pthread_create(&vis_thr, NULL, start_2d_visual, (void*) run_name);

    per_thread = box->num_particles / NUM_THREADS;
    remaining = box->num_particles % NUM_THREADS;
    curr_index = 0;
    for(i=0; i<NUM_THREADS; i++) {
        jobs[i].seed = (Seed) myrand_int(&s);
        jobs[i].box = box;
        jobs[i].dt = TIMESTEP;

        jobs[i].first_index = curr_index;
        curr_index += per_thread;
        if(remaining > 0) {
            curr_index++;
            remaining--;
        }
        jobs[i].last_index = curr_index;
    }
    workers = thr_pool_create(NUM_THREADS, NUM_THREADS, 5, NULL);

    sprintf(filename, "%s_Analysis.txt", run_name);
    out = fopen(filename, "w");

    elapsed_force = 0;
    elapsed_run = 0;
    init_velocities(box, &s);
    for(i=0; i<NUM_THREADS; i++) {
        thr_pool_queue(workers, update_interact, &jobs[i]);
    }
    thr_pool_wait(workers);
    for(k=0; k<=num_steps; k++) {
        analyze(box, run_name, k, out);
        if(output_png) {
            sprintf(filename, "%s_%08d.png", run_name, k);
            take_sshot(filename);
        }

        // velocity (t + 1/2 dt)    (update_velocity)
        // position (t + dt)        (update_position)
        // force    (t + dt)        (update_interact)
        // velocity (t + dt)        (update_velocity)

        gettimeofday(&start_run, NULL);
        fprintf(stderr, "time step %d: updating velocities (first half)\n", k);
        for(i=0; i<NUM_THREADS; i++) {
            thr_pool_queue(workers, update_velocity, &jobs[i]);
        }
        thr_pool_wait(workers);

        fprintf(stderr, "time step %d: updating position\n", k);
        update_position(box, TIMESTEP);

        gettimeofday(&start_force, NULL);
        fprintf(stderr, "time step %d: updating forces\n", k);
        for(i=0; i<NUM_THREADS; i++) {
            thr_pool_queue(workers, update_interact, &jobs[i]);
        }
        thr_pool_wait(workers);
        gettimeofday(&end_force, NULL);
        elapsed_force += time_elapsed(&start_force, &end_force);

        fprintf(stderr, "time step %d: updating velocities\n", k);
        for(i=0; i<NUM_THREADS; i++) {
            thr_pool_queue(workers, update_velocity, &jobs[i]);
        }
        thr_pool_wait(workers);
        gettimeofday(&end_run, NULL);
        elapsed_run += time_elapsed(&start_run, &end_run);
    }

    fprintf(out,"\nProportion execution time calculating forces: %f\n",
        ((double)elapsed_force/(double)elapsed_run));
    fclose(out);

    // cleanup stuff (optional)
    delete_2d_openbox(box);
    thr_pool_destroy(workers);

    return 0;
}


int main(int argc, char **argv)
{
    int num_steps;
    char output_png;
    char *run_name;

    if(argc < 4) {
        printf("Usage: %s <dt> <num steps> <run name> [output png]\n", argv[0]);
        return -1;
    }

    TIMESTEP = strtod(argv[1], 0);
    num_steps = strtol(argv[2], 0, 10);
    run_name = argv[3];
    fprintf(stdout, "Time step: %f\nNumber of steps: %d\nRun Name: %s\n",
            TIMESTEP, num_steps, run_name);

    if(argc >= 5) {
        output_png = 1;
    } else {
        output_png = 0;
    }

    start_md(num_steps, output_png, run_name);

    return 0;   /* ANSI C requires main to return int. */
}
