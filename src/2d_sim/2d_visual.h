#ifndef _2D_VISUAL_H_
#define _2D_VISUAL_H_

#include <GL/glut.h>
#include "2d_openbox.h"


#define NUM_DIMS    2
#define NEDGES      4
#define NVERTS      4


typedef struct {
    // (half_width, half_height, half_depth)
    GLdouble half_dims[NUM_DIMS];
    // GLdouble clip_eqns[NEDGES][4];
    GLdouble radius;
    GLfloat particle_color[4];

    OpenBox2D *box;
    // QTree *dbg_state;
    // GLfloat xrot;
    // GLfloat yrot;
    char dbg_mode;
} VisualContext2D;


VisualContext2D* new_2d_visual(OpenBox2D * const box);
VisualContext2D* set_2d_visual(VisualContext2D * const vc);
void* start_2d_visual(void* data);
int take_sshot(char * const filepath);

// debug stuff
// void display_octree(void);

#endif
