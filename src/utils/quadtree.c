
// bastardized implementation of quadtree
// assumes point elements, so each element only has one parent leaf


#include <stdio.h>
#include <stdlib.h>

#include "quadtree.h"


#define MAX_NUM_POINTS  4
#define MAX_DEPTH       50

// (x, y) = (min_x, min_y) is the upper left
// (x, y) = (max_x, max_y) is the bottom right
QTree* new_quadtree(const double y_min, const double y_max,
                    const double x_min, const double x_max)
{
    QTree *tree;

    tree = (QTree*) malloc(sizeof(QTree));

    tree->y_min = y_min;
    tree->y_max = y_max;
    tree->x_min = x_min;
    tree->x_max = x_max;
    tree->y_mid = (y_max + y_min)/2.0;
    tree->x_mid = (x_max + x_min)/2.0;
    tree->upper_left = NULL;
    tree->upper_right = NULL;
    tree->lower_left = NULL;
    tree->lower_right = NULL;
    tree->num_points = 0;
    tree->head = NULL;
    tree->tail = NULL;
    tree->depth = 0;

    return tree;
}

int quadtree_split(QTree * const tree)
{
    int i;
    QTree *ul, *ur, *ll, *lr;
    ListNode *cn;   /* current node pointer */
    QTree *next_tree;
    int res;

    // debug
    // fprintf(stderr,"splitting quadtree (%f, %f) (%f, %f)\n",
        // tree->x_min, tree->y_min, tree->x_max, tree->y_max);

    if(tree->depth+1 > MAX_DEPTH) {
        return 1;
    }

    ul = new_quadtree(tree->y_min, tree->y_mid, tree->x_min, tree->x_mid);
    ur = new_quadtree(tree->y_min, tree->y_mid, tree->x_mid, tree->x_max);
    ll = new_quadtree(tree->y_mid, tree->y_max, tree->x_min, tree->x_mid);
    lr = new_quadtree(tree->y_mid, tree->y_max, tree->x_mid, tree->x_max);

    for(i=0; i<tree->num_points; i++) {
        cn = tree->head;
        tree->head = tree->head->next;

        if(cn->y_pos <= tree->y_mid && cn->x_pos <= tree->x_mid) {
            next_tree = ul;
        } else if (cn->y_pos <= tree->y_mid && cn->x_pos > tree->x_mid) {
            next_tree = ur;
        } else if (cn->y_pos > tree->y_mid && cn->x_pos <= tree->x_mid) {
            next_tree = ll;
        } else {
            next_tree = lr;
        }

        if(next_tree->tail != NULL)
            next_tree->tail->next = cn;
        next_tree->tail = cn;
        if(next_tree->head == NULL)
            next_tree->head = cn;
        cn->next = NULL;

        next_tree->num_points++;
    }

    tree->num_points = 0;
    tree->head = NULL;
    tree->tail = NULL;
    tree->upper_left = ul;
    tree->upper_right = ur;
    tree->lower_left = ll;
    tree->lower_right = lr;
    for(i=0; i<NUM_QUADRANTS; i++) {
        tree->quadrants[i]->depth = tree->depth + 1;
    }

    res = 0;
    if(tree->depth+1+1 <= MAX_DEPTH) {
        for(i=0; i<NUM_QUADRANTS; i++) {
            if(tree->quadrants[i]->num_points > MAX_NUM_POINTS) {
                res |= quadtree_split(tree->quadrants[i]);
            }
        }
    }

    return res;
}

#if 0
int quadtree_has(QTree * const tree, const double y, const double x)
{
    int i;
    int found = 0;
    ListNode *cn;   /* current node pointer */

    cn = tree->head;
    for(i=0; i<tree->num_points; i++) {
        if(cn->y_pos == y && cn->x_pos == x) {
            found = 1;
            break;
        }

        cn = cn->next;
    }

    if(!found) {
        if(y <= tree->y_mid && x <= tree->x_mid && tree->upper_left != NULL) {
            found = quadtree_has(tree->upper_left, y, x);
        } else if (y <= tree->y_mid && x > tree->x_mid && tree->upper_right != NULL) {
            found = quadtree_has(tree->upper_right, y, x);
        } else if (y > tree->y_mid && x <= tree->x_mid && tree->lower_left != NULL) {
            found = quadtree_has(tree->lower_left, y, x);
        } else if (y > tree->y_mid && x > tree->x_mid && tree->lower_right != NULL) {
            found = quadtree_has(tree->lower_right, y, x);
        }
    }

    return found;
}
#endif

// add some data to the tree
ListNode* quadtree_add(QTree * const tree, const double y, const double x,
                       void * const data)
{
    if(tree->upper_left == NULL || tree->upper_right == NULL ||
       tree-> lower_left == NULL || tree->lower_right == NULL) {
        if(tree->depth >= MAX_DEPTH || tree->num_points < MAX_NUM_POINTS) {
            // then add data point to the current node
            // if(quadtree_has(tree, y, x)) {
                // fprintf(stderr, "attempt to add same point failed!\n");
                // return NULL;
            // }

            // fprintf(stderr,"adding point (%f, %f)\n",x,y);

            ListNode *new_point = (ListNode*) malloc(sizeof(ListNode));
            new_point->y_pos = y;
            new_point->x_pos = x;
            new_point->data = data;
            new_point->next = NULL;

            if(tree->tail != NULL)
                tree->tail->next = new_point;
            tree->tail = new_point;
            if(tree->head == NULL)
                tree->head = new_point;

            tree->num_points++;
            return new_point;
        } else {
            // split the current node
            quadtree_split(tree);
        }
    }

    // traverse the tree
    if(y <= tree->y_mid && x <= tree->x_mid) {
        return quadtree_add(tree->upper_left, y, x, data);
    } else if (y <= tree->y_mid && x > tree->x_mid) {
        return quadtree_add(tree->upper_right, y, x, data);
    } else if (y > tree->y_mid && x <= tree->x_mid) {
        return quadtree_add(tree->lower_left, y, x, data);
    } else {
        return quadtree_add(tree->lower_right, y, x, data);
    }
}

int delete_quadtree(QTree * const tree)
{
    if(tree->num_points > 0) {
        int i;
        ListNode *cn;

        for(i=0; i<tree->num_points; i++) {
            cn = tree->head;
            tree->head = tree->head->next;
            free(cn);
        }
    }

    if(tree->upper_left != NULL) {
        delete_quadtree(tree->upper_left);
    }
    if(tree->upper_right != NULL) {
        delete_quadtree(tree->upper_right);
    }
    if(tree->lower_left != NULL) {
        delete_quadtree(tree->lower_left);
    }
    if(tree->lower_right != NULL) {
        delete_quadtree(tree->lower_right);
    }

    free(tree);

    return 0;
}


#if 0

// for debugging purposes
void quadtree_dump(QTree * const tree, Frame * const frame, char * const filename,
                   Seed * const s, int level)
{
    int i, j;
    int height, width;
    Frame *f;
    Pixel white;
    Pixel rand_color;

    white.r = 255;
    white.g = 255;
    white.b = 255;
    rand_color.r = 255*(myrand_double(s));
    rand_color.g = 255*(myrand_double(s));
    rand_color.b = 255*(myrand_double(s));

    height = (int) (tree->y_max - tree->y_min + 0.5);
    width = (int) (tree->x_max - tree->x_min + 0.5);

    // fprintf(stderr,"quadtree (%f, %f) (%f, %f) [%d, %d]\n",
    // tree->x_min, tree->y_min, tree->x_max, tree->y_max, height, width);

    if(frame == NULL) {
        // fprintf(stderr,"\tdumping new frame, color code: (%d, %d, %d)\n",
        // rand_color.r, rand_color.g, rand_color.b);

        f = new_frame(height,width);

        // setting background to be white
        for(i=0; i<f->height; i++) {
            for(j=0; j<f->width; j++) {
                set_pixel(f, i+(int)tree->y_min, j+(int)tree->x_min, &white);
            }
        }
    } else {
        // fprintf(stderr,"\tentering sub frame level %d, color code: (%d, %d, %d)\n",
        // level, rand_color.r, rand_color.g, rand_color.b);
        f = frame;
    }

    // drawing border
    for(i=0; i<height; i++) {
        set_pixel(f, i+(int)tree->y_min, 0+(int)tree->x_min, &rand_color);
        set_pixel(f, i+(int)tree->y_min, width-1+(int)tree->x_min, &rand_color);
    }
    for(j=0; j<width; j++) {
        set_pixel(f, 0+(int)tree->y_min, j+(int)tree->x_min, &rand_color);
        set_pixel(f, height-1+(int)tree->y_min, j+(int)tree->x_min, &rand_color);
    }

    if(tree->upper_left == NULL || tree->upper_right == NULL ||
       tree-> lower_left == NULL || tree->lower_right == NULL) {
        ListNode *cn;   /* current node pointer */
        cn = tree->head;
        for(i=0; i<tree->num_points; i++) {
            fill_circle(f, (int)(cn->y_pos+0.5), (int)(cn->x_pos+0.5), 10, &rand_color);
            cn = cn->next;
        }
    } else {
        quadtree_dump(tree->upper_left, f, NULL, s, level+1);
        quadtree_dump(tree->upper_right, f, NULL, s, level+1);
        quadtree_dump(tree->lower_left, f, NULL, s, level+1);
        quadtree_dump(tree->lower_right, f, NULL, s, level+1);
    }

    if(filename != NULL)
        write_frame(f, filename);
}

#endif
