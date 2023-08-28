#ifndef _3D_VISUAL_H_
#define _3D_VISUAL_H_

#include <GL/glut.h>
#include "3d_openbox.h"


#define NUM_DIMS    3
#define NFACE       6
#define NVERT       8


typedef struct {
    // (half_width, half_height, half_depth)
    GLdouble half_dims[NUM_DIMS];
    GLfloat vert_colors[NVERT][4];
    GLdouble clip_eqns[NFACE][4];

    GLdouble radius;
    GLfloat particle_color[4];

    OpenBox *box;
    GLfloat xrot;
    GLfloat yrot;
    char dbg_mode;
} VisualContext;


VisualContext* new_visual(OpenBox * const box);
VisualContext* set_visual(VisualContext * const vc);
void* start_visual(void* data);
int take_sshot(char * const filepath);


// debug stuff
// void display_octree(void);

#endif
