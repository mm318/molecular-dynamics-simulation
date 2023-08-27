#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <unistd.h>
#include <semaphore.h>

#include "3d_visual.h"
#include "../lib/png_writer.h"

#ifndef M_PI
#define M_PI 3.14159265
#endif


static Seed s;

static const GLenum clip_planes[NFACE] = {
    GL_CLIP_PLANE0, GL_CLIP_PLANE1, GL_CLIP_PLANE2,
    GL_CLIP_PLANE3, GL_CLIP_PLANE4, GL_CLIP_PLANE5
};

static VisualContext *cc;   // current context

static const GLfloat def_vert_colors[NVERT][4] = {
    {0.0, 0.0, 0.0, 0.05}, {1.0, 0.0, 0.0, 0.05},
    {0.0, 1.0, 0.0, 0.05}, {1.0, 1.0, 0.0, 0.05},
    {0.0, 0.0, 1.0, 0.05}, {1.0, 0.0, 1.0, 0.05},
    {0.0, 1.0, 1.0, 0.05}, {1.0, 1.0, 1.0, 0.05}
};

static const GLdouble def_clip_eqns[NFACE][3] = {
    {-1.0, 0.0, 0.0}, {1.0, 0.0, 0.0},
    {0.0, -1.0, 0.0}, {0.0, 1.0, 0.0},
    {0.0, 0.0, -1.0}, {0.0, 0.0, 1.0}
};

static const GLfloat def_particle_color[4] = {1.0, 0.0, 0.0, 0.25};

VisualContext* new_visual(OpenBox * const box)
{
    int i, j;
    VisualContext *vc = (VisualContext*) malloc(sizeof(VisualContext));

    // setting default values for visual context
    vc->half_dims[0] = box->width/2.0;
    vc->half_dims[1] = box->height/2.0;
    vc->half_dims[2] = box->depth/2.0;

    for(i=0; i<NVERT; i++) {
        for(j=0; j<3; j++) {
            vc->vert_colors[i][j] = myrand_double(&s);
        }
        vc->vert_colors[i][3] = def_vert_colors[i][3];
    }

    for(i=0; i<NFACE; i++) {
        for(j=0; j<3; j++) {
            vc->clip_eqns[i][j] = def_clip_eqns[i][j];
        }
        vc->clip_eqns[i][3] = vc->half_dims[i/2];
    }

    vc->radius = RADIUS;
    for(i=0; i<4; i++) {
        vc->particle_color[i] = def_particle_color[i];
    }

    vc->box = box;
    vc->xrot = 45;
    vc->yrot = 45;
    vc->dbg_mode = 0;   // initially dbg_mode off

    return vc;
}

VisualContext* set_visual(VisualContext * const vc)
{
    VisualContext *ret = cc;
    cc = vc;
    // glutPostRedisplay();
    return ret;
}

static void init_gl(void)
{
    // Note:  polygons must be drawn from front to back
    // for proper blending if culling is enabled
    // glCullFace(GL_BACK);
    glDisable(GL_CULL_FACE);
    // glBlendFunc (GL_SRC_ALPHA_SATURATE, GL_ONE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor (1.0, 1.0, 1.0, 0.0);

    glEnable(GL_BLEND);
    glEnable(GL_POLYGON_SMOOTH);
    glDisable(GL_DEPTH_TEST);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT,GL_NICEST);

    glDisable(GL_TEXTURE_2D);
    glEnable(GL_DITHER);

    // lighting for bringing out 3d spheres
    // glEnable(GL_LIGHTING);
    // glLightfv(GL_LIGHT0, GL_POSITION, light0_pos);
    GLfloat lightPos0[4] = {0.0f, 0.0f, 0.0f, 0};
    GLfloat lightCol0[4] = {1.0f, 1.0f, 1.0f, 0};
    GLfloat lightSpec[4] = {0.5f, 0.5f, 0.5f, 0};
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos0);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightCol0);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpec);
    glEnable(GL_LIGHT0);
}


void drawCube(GLdouble x0, GLdouble x1, GLdouble y0, GLdouble y1,
              GLdouble z0, GLdouble z1)
{
    /*  indices of front, top, left, bottom, right, back faces  */
    static const GLubyte vert_indices[NFACE][4] = {
        {4, 5, 6, 7}, {2, 3, 7, 6}, {0, 4, 7, 3},
        {0, 1, 5, 4}, {1, 5, 6, 2}, {0, 3, 2, 1}
    };

    GLfloat v[NVERT][3];

    v[0][0] = v[3][0] = v[4][0] = v[7][0] = x0;
    v[1][0] = v[2][0] = v[5][0] = v[6][0] = x1;
    v[0][1] = v[1][1] = v[4][1] = v[5][1] = y0;
    v[2][1] = v[3][1] = v[6][1] = v[7][1] = y1;
    v[0][2] = v[1][2] = v[2][2] = v[3][2] = z0;
    v[4][2] = v[5][2] = v[6][2] = v[7][2] = z1;

#ifdef GL_VERSION_1_1
    glEnableClientState (GL_VERTEX_ARRAY);
    glEnableClientState (GL_COLOR_ARRAY);
    glVertexPointer (3, GL_FLOAT, 0, v);
    glColorPointer (4, GL_FLOAT, 0, cc->vert_colors);
    glDrawElements (GL_QUADS, NFACE*4, GL_UNSIGNED_BYTE, vert_indices);
    glDisableClientState (GL_VERTEX_ARRAY);
    glDisableClientState (GL_COLOR_ARRAY);
#else
    fprintf(stderr, "If this is GL Version 1.0, ");
    fprintf(stderr, "vertex arrays are not supported.\n");
    // exit(1);
#endif
}

void draw_particle(GLfloat x, GLfloat y, GLfloat z)
{
    int i;
    int num_repeat;
    double x_pos[8], y_pos[8], z_pos[8];

    num_repeat = find_repeats(cc->box, x, y, z, RADIUS, x_pos, y_pos, z_pos);

    for(i=0; i<num_repeat; i++) {
        glPushMatrix();
        // glTranslatef(x_pos[i] - cc->half_dims[0], y_pos[i] - cc->half_dims[1],
                     // z_pos[i] - cc->half_dims[2]);
        glTranslatef(x_pos[i], y_pos[i], z_pos[i]);
        glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, cc->particle_color);
        glutSolidSphere(cc->radius, 100, 100);
        glPopMatrix();
    }
}

void display_octree();  // declaration of debug function

void display()
{
    int i;
    ListNode *cn;
    OpenBox *box = cc->box;

    if(cc->dbg_mode) {
        display_octree();
        return;
    }

    glClear(GL_COLOR_BUFFER_BIT);

    // setting up scene view
    glPushMatrix();
    glRotatef (cc->yrot, 1.0, 0.0, 0.0);
    glRotatef (cc->xrot, 0.0, 1.0, 0.0);

    // drawing particles
    // need to enable lighting to bring out 3d effects
    glEnable(GL_LIGHTING);
    // // enable clipping planes so particles don't leave box
    for(i=0; i<NFACE; i++) {
        glClipPlane(clip_planes[i], cc->clip_eqns[i]);
        glEnable(clip_planes[i]);
    }

    // draw particles here
    glPushMatrix();
    glTranslatef(-cc->half_dims[0], -cc->half_dims[1], -cc->half_dims[2]);
    for(i=0; i<box->num_particles; i++) {
        cn = box->particles[i];
        draw_particle(cn->pos.x, cn->pos.y, cn->pos.z);
    }
    glPopMatrix ();

    glDisable(GL_LIGHTING);
    for(i=0; i<NFACE; i++) {
        glDisable(clip_planes[i]);
    }

    // drawing containment box
    glPushMatrix();
    drawCube(-cc->half_dims[0], cc->half_dims[0], -cc->half_dims[1],
             cc->half_dims[1], -cc->half_dims[2], cc->half_dims[2]);
    glPopMatrix ();

    glPopMatrix();
    glFlush();
    glutSwapBuffers();
}


static int curr_x, curr_y;
static int button_down = 0;

void mouse(int button, int state, int x, int y)
{
    if(button == GLUT_LEFT_BUTTON) {
        button_down = (state == GLUT_DOWN);
    }

    if(button_down) {
        curr_x = x;
        curr_y = y;
    }
}

void mouseMove(int x, int y)
{
    int dx, dy;

    if(button_down) {
        dx = x - curr_x;
        dy = y - curr_y;

        curr_x = x;
        curr_y = y;

        cc->xrot += dx;
        cc->yrot += dy;

        glutPostRedisplay();
    }

    usleep(1000);
}

// void keyboard(unsigned char key, int x, int y)
// {
// if(key == 27) {
// // escape key pressed
// exit(0);
// }
// }


static int W, H;

void reshape(int width, int height)
{
    int i;
    GLint x, y;
    GLint view_dim;
    GLdouble extent, max_dim = -99999999;

    if(width > height) {
        view_dim = height;
    } else {
        view_dim = width;
    }

    x = (width - view_dim)/2;
    y = (height - view_dim)/2;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glViewport(x, y, (GLint) view_dim, (GLint) view_dim);

    // double aspect, top, bottom, left, right;
    // // aspect = (double) width / (double) height;
    // aspect = 1;
    // top = tan(FOV * M_PI / 360.0 ) * NEARCLIP;
    // bottom = -top;
    // left = aspect*top;
    // right = -left;
    // glFrustum(left, right, top, bottom, NEARCLIP, FARCLIP);

    // glOrtho(-2*BOX_HALF_DIM, 2*BOX_HALF_DIM, -2*BOX_HALF_DIM,
    // 2*BOX_HALF_DIM, -2*BOX_HALF_DIM, 2*BOX_HALF_DIM);

    for(i=0; i<NUM_DIMS; i++) {
        if(cc->half_dims[i] > max_dim) {
            max_dim = cc->half_dims[i];
        }
    }
    extent = sqrt(3)*max_dim;
    glOrtho(-extent, extent, -extent, extent, -extent, extent);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    W = width;
    H = height;

    glutPostRedisplay();
}

static bitmap_t* get_bitmap(const int width, const int height)
{
    int i, j;
    GLubyte *image, *p;
    bitmap_t *bmp;

    image = (GLubyte *) malloc(width * height * sizeof(GLubyte) * 3);

    /* OpenGL's default 4 byte pack alignment would leave extra bytes at the
       end of each image row so that each full row contained a number of bytes
       divisible by 4.  Ie, an RGB row with 3 pixels and 8-bit componets would
       be laid out like "RGBRGBRGBxxx" where the last three "xxx" bytes exist
       just to pad the row out to 12 bytes (12 is divisible by 4). To make sure
       the rows are packed as tight as possible (no row padding), set the pack
       alignment to 1. */
    glPixelStorei(GL_PACK_ALIGNMENT, 1);

    glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, image);

    p = image;
    bmp = new_bitmap(height, width);
    for (i = height-1; i >= 0; i--) {
        for(j=0; j<width; j++) {
            bmp->pixels[i*width+j].red = p[0];
            bmp->pixels[i*width+j].green = p[1];
            bmp->pixels[i*width+j].blue = p[2];
            p += 3*sizeof(GLubyte);
        }
    }
    free(image);
    return bmp;
}
int write_png(char *filename, const int width, const int height)
{
    bitmap_t *bmp;
    bmp = get_bitmap(width, height);
    write_bitmap(bmp, filename);
    delete_bitmap(bmp);
    return 0;
}


static sem_t sshot_wait;
static char *sshot_file;

int take_sshot(char * const filepath)
{
    cc->xrot += 0.2;
    cc->yrot += 0.3;

    sshot_file = filepath;
    sem_wait(&sshot_wait);  // 0 = sshot not ready, 1 = ready
    return 0;
}

void check_sshot()
{
    int ret_val, sem_val;

    ret_val = sem_getvalue(&sshot_wait, &sem_val);

    // POSIX.1-2001 permits TWO (WTF!) possibilities for the value returned
    // in sval: either 0 is returned; or a negative number whose absolute
    // value is the count of the number of processes and threads currently
    // blocked in sem_wait(3). Linux adopts the former behavior.

    if(ret_val ==0 && sem_val <= 0) {
        glutPostRedisplay();
        write_png(sshot_file, W, H);
        sem_post(&sshot_wait);
    }

    // usleep(200*1000);   // to prevent too much cpu usage
}

static void menu(int value)
{
    switch(value) {
    case 0:
        write_png("debug_out.png", W, H);
        break;
    case 1:
        cc->dbg_mode = !cc->dbg_mode;
        glutPostRedisplay();
        break;
    }
}

void* start_visual(void* data)
{
    char *title = (char*) data;
    int arg_count = 0;
    s = (Seed) time(NULL);

    glutInit(&arg_count, NULL);
    glutInitDisplayMode (GLUT_SINGLE|GLUT_RGB|GLUT_ALPHA|GLUT_DEPTH);
    glutInitWindowSize(650, 650);
    sem_init(&sshot_wait, 0, 0);

    glutCreateWindow(title);
    init_gl();

    glutReshapeFunc(reshape);
    glutMouseFunc(mouse);
    glutMotionFunc(mouseMove);
    // glutKeyboardFunc(keyboard);
    glutDisplayFunc(display);
    // glutDisplayFunc(display_octree);
    // glutTimerFunc(REFRESH_RATE, animate, 0);
    glutIdleFunc(check_sshot);

    glutCreateMenu(menu);
    glutAddMenuEntry("Write PNG", 0);
    glutAddMenuEntry("Toggle Debug Mode", 1);
    glutAttachMenu(GLUT_RIGHT_BUTTON);

    glutMainLoop();

    return NULL;
}


// debug stuff
#if 0
void test_particles()
{
    int i;
    static const GLfloat debug_color[4] = {1.0, 0.0, 0.0, 1.0};
    GLfloat face_centers[NFACE][3] = {
        {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0},
        {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0},
        {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}
    };

    for(i=0; i<NFACE; i++) {
        if(i%2 == 0) {
            face_centers[i][i/2] = (GLfloat) cc->half_dims[i/2];
        } else {
            face_centers[i][i/2] = (GLfloat) -cc->half_dims[i/2];
        }
    }

    for(i=0; i<NFACE; i++) {
        glPushMatrix();
        glTranslatef(face_centers[i][0], face_centers[i][1], face_centers[i][2]);
        glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, debug_color);
        glutSolidSphere(cc->radius, 100, 100);
        glPopMatrix();
    }
}
#endif

void draw_octant(Octree * const tree)
{
    int i;
    ListNode *cn;   /* current node pointer */

    for(i=0; i<NUM_OCTANTS; i++) {
        if(tree->octants[i] != NULL) {
            draw_octant(tree->octants[i]);
        }
    }

    // drawing particles
    glEnable(GL_LIGHTING);
    for(i=0; i<NFACE; i++) {
        glClipPlane(clip_planes[i], cc->clip_eqns[i]);
        glEnable(clip_planes[i]);
    }

    cn = tree->head;
    for(i=0; i<tree->num_points; i++) {
        glPushMatrix();
        glTranslatef(cn->pos.x, cn->pos.y, cn->pos.z);
        glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, cc->particle_color);
        glutSolidSphere(cc->radius, 100, 100);
        glPopMatrix();

        cn = cn->next;
    }

    glDisable(GL_LIGHTING);
    for(i=0; i<NFACE; i++) {
        glDisable(clip_planes[i]);
    }

    // drawing containment box
    glPushMatrix();
    // GLdouble dim = tree->max.x - tree->min.x;
    // glTranslatef(tree->mid.x, tree->mid.y, tree->mid.z);
    // glutWireCube(dim);
    drawCube(tree->min.x, tree->max.x, tree->min.y,
             tree->max.y, tree->min.z, tree->max.z);
    glPopMatrix ();
}

void display_octree()
{
    int i;

    glClear(GL_COLOR_BUFFER_BIT);

    // setting up scene view
    glPushMatrix();
    glRotatef (cc->yrot, 1.0, 0.0, 0.0);
    glRotatef (cc->xrot, 0.0, 1.0, 0.0);

    draw_octant(cc->box->tree);

    // drawing test particles
    glEnable(GL_LIGHTING);
    for(i=0; i<NFACE; i++) {
        glClipPlane(clip_planes[i], cc->clip_eqns[i]);
        glEnable(clip_planes[i]);
    }
    // test_particles();
    glDisable(GL_LIGHTING);
    for(i=0; i<NFACE; i++) {
        glDisable(clip_planes[i]);
    }

    glPopMatrix();
    glFlush();
    glutSwapBuffers();
}
