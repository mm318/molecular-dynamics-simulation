#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <unistd.h>
#include <semaphore.h>

#include "2d_visual.h"
#include "utils/my_rand.h"
#include "utils/png_writer.h"

#ifndef M_PI
#define M_PI 3.14159265
#endif


static Seed s;

static VisualContext2D *cc;   // current context

static const GLfloat def_particle_color[4] = {1.0, 0.0, 0.0, 1.0};

VisualContext2D* new_2d_visual(OpenBox2D * const box)
{
    int i;
    VisualContext2D *vc = (VisualContext2D*) malloc(sizeof(VisualContext2D));

    // setting default values for visual context
    vc->half_dims[0] = box->width/2.0;
    vc->half_dims[1] = box->height/2.0;

    vc->radius = RADIUS;
    for(i=0; i<4; i++) {
        vc->particle_color[i] = def_particle_color[i];
    }

    vc->box = box;
    vc->dbg_mode = 0;   // initially dbg_mode off

    return vc;
}

VisualContext2D* set_2d_visual(VisualContext2D * const vc)
{
    VisualContext2D *ret = cc;
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
    GLfloat lightPos0[4] = {-100.0f, 100.0f, 15.0f, 0};
    GLfloat lightCol0[4] = {1.0f, 1.0f, 1.0f, 0};
    GLfloat lightSpec[4] = {0.5f, 0.5f, 0.5f, 0};
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos0);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightCol0);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpec);
    glEnable(GL_LIGHT0);
}


void draw_rectangle(GLdouble x0, GLdouble x1, GLdouble y0, GLdouble y1)
{
    glBegin(GL_LINE_LOOP);
    glVertex2f(x0, y0);
    glVertex2f(x1, y0);
    glVertex2f(x1, y1);
    glVertex2f(x0, y1);
    glEnd();
}

void draw_particle(GLfloat x, GLfloat y)
{
    int i;
    int num_repeat;
    double x_pos[4], y_pos[4];

    num_repeat = find_repeats(cc->box, x, y, RADIUS, x_pos, y_pos);

    for(i=0; i<num_repeat; i++) {
        glPushMatrix();
        glTranslatef(x_pos[i], y_pos[i], 0);
        glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, cc->particle_color);
        glutSolidSphere(cc->radius, 100, 100);
        glPopMatrix();
    }
}

void display_quadtree();  // declaration of debug function

void display()
{
    int i;
    ListNode *cn;
    OpenBox2D *box = cc->box;

    if(cc->dbg_mode) {
        display_quadtree();
        return;
    }

    glClear(GL_COLOR_BUFFER_BIT);

    // setting up scene view
    glPushMatrix();
    glTranslatef(-cc->box->width/2.0, -cc->box->height/2.0, 0);

    // drawing particles
    // need to enable lighting to bring out 3d effects
    glEnable(GL_LIGHTING);
    for(i=0; i<box->num_particles; i++) {
        cn = box->particles[i];
        draw_particle(cn->x_pos, cn->y_pos);
    }

    glDisable(GL_LIGHTING);

    // glPushMatrix();
    // glColor3b(0, 0, 0);
    // draw_rectangle(0, cc->box->width, 0, cc->box->height);
    // glPopMatrix ();

    glPopMatrix();
    glFlush();
    glutSwapBuffers();
}


static int W, H;

void reshape(int width, int height)
{
    int i;
    GLint x, y;
    GLint view_dim;
    GLdouble max_dim = -99999999;

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
    for(i=0; i<NUM_DIMS; i++) {
        if(cc->half_dims[i] > max_dim) {
            max_dim = cc->half_dims[i];
        }
    }
    // max_dim += 0.375;
    glOrtho(-max_dim, max_dim, -max_dim, max_dim, -RADIUS-5, RADIUS+5);
    // glDisable(GL_DEPTH_TEST)

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    // glTranslatef(0.375, 0.375, 0);  // pixel alignment trick

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
    switch( value ) {
    case 0:
        write_png("debug_out.png", W, H);
        break;
    case 1:
        cc->dbg_mode = !cc->dbg_mode;
        glutPostRedisplay();
        break;
    }
}

void* start_2d_visual(void* data)
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
    // glutMouseFunc(mouse);
    // glutMotionFunc(mouseMove);
    glutDisplayFunc(display);
    // glutDisplayFunc(display_quadtree);
    glutIdleFunc(check_sshot);

    glutCreateMenu(menu);
    glutAddMenuEntry("Write PNG", 0);
    glutAddMenuEntry("Toggle Debug Mode", 1);
    glutAttachMenu(GLUT_RIGHT_BUTTON);

    glutMainLoop();

    return NULL;
}


// debug stuff

void draw_quadrant(QTree * const tree)
{
    int i;
    ListNode *cn;   /* current node pointer */
    GLbyte r, g, b;

    for(i=0; i<NUM_QUADRANTS; i++) {
        if(tree->quadrants[i] != NULL) {
            draw_quadrant(tree->quadrants[i]);
        }
    }

    // drawing particles
    glEnable(GL_LIGHTING);
    cn = tree->head;
    for(i=0; i<tree->num_points; i++) {
        draw_particle(cn->x_pos, cn->y_pos);
        cn = cn->next;
    }
    glDisable(GL_LIGHTING);

    // drawing containment box
    glPushMatrix();
    r = 255*(myrand_double(&s));
    g = 255*(myrand_double(&s));
    b = 255*(myrand_double(&s));
    glColor3b(r, g, b);
    draw_rectangle(tree->x_min, tree->x_max, tree->y_min, tree->y_max);
    glPopMatrix ();
}

void display_quadtree()
{
    glClear(GL_COLOR_BUFFER_BIT);

    // setting up scene view
    glPushMatrix();
    glTranslatef(-cc->box->width/2.0, -cc->box->height/2.0, 0);
    draw_quadrant(cc->box->tree);
    glPopMatrix();

    glFlush();
    glutSwapBuffers();
}
