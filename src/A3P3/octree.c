
// bastardized implementation of octree// assumes point elements, so each element only has one parent leaf


#include <stdio.h>
#include <stdlib.h>

#include "octree.h"


#define MAX_NUM_POINTS  8
#define MAX_DEPTH       50


Octree* new_octree(const double x_min, const double x_max, const double y_min,
                   const double y_max, const double z_min, const double z_max)
{
    int i;
    Octree *tree = (Octree*) malloc(sizeof(Octree));

    tree->min.x = x_min;
    tree->min.y = y_min;
    tree->min.z = z_min;

    tree->max.x = x_max;
    tree->max.y = y_max;
    tree->max.z = z_max;

    tree->mid.x = (x_min + x_max)/2.0;
    tree->mid.y = (y_min + y_max)/2.0;
    tree->mid.z = (z_min + z_max)/2.0;

    for(i=0; i<NUM_OCTANTS; i++) {
        tree->octants[i] = NULL;
    }

    tree->num_points = 0;
    tree->head = NULL;
    tree->tail = NULL;
    tree->depth = 0;

    return tree;
}

int octree_split(Octree * const t)
{
    int i;
    Octree *new_trees[NUM_OCTANTS];
    ListNode *cn;   /* current node pointer */
    Octree *next_tree;
    int res;

    if(t->depth+1 > MAX_DEPTH) {
        return 1;
    }

    new_trees[LEFT_UP_NEAR] =
        new_octree(t->min.x, t->mid.x, t->mid.y, t->max.y, t->mid.z, t->max.z);
    new_trees[RIGHT_UP_NEAR] =
        new_octree(t->mid.x, t->max.x, t->mid.y, t->max.y, t->mid.z, t->max.z);
    new_trees[LEFT_DOWN_NEAR] =
        new_octree(t->min.x, t->mid.x, t->min.y, t->mid.y, t->mid.z, t->max.z);
    new_trees[RIGHT_DOWN_NEAR] =
        new_octree(t->mid.x, t->max.x, t->min.y, t->mid.y, t->mid.z, t->max.z);
    new_trees[LEFT_UP_FAR] =
        new_octree(t->min.x, t->mid.x, t->mid.y, t->max.y, t->min.z, t->mid.z);
    new_trees[RIGHT_UP_FAR] =
        new_octree(t->mid.x, t->max.x, t->mid.y, t->max.y, t->min.z, t->mid.z);
    new_trees[LEFT_DOWN_FAR] =
        new_octree(t->min.x, t->mid.x, t->min.y, t->mid.y, t->min.z, t->mid.z);
    new_trees[RIGHT_DOWN_FAR] =
        new_octree(t->mid.x, t->max.x, t->min.y, t->mid.y, t->min.z, t->mid.z);

    for(i=0; i<t->num_points; i++) {
        cn = t->head;
        t->head = t->head->next;

        if(cn->pos.x <= t->mid.x && cn->pos.y > t->mid.y && cn->pos.z > t->mid.z) {
            next_tree = new_trees[LEFT_UP_NEAR];
        } else if(cn->pos.x > t->mid.x && cn->pos.y > t->mid.y && cn->pos.z > t->mid.z) {
            next_tree = new_trees[RIGHT_UP_NEAR];
        } else if(cn->pos.x <= t->mid.x && cn->pos.y <= t->mid.y && cn->pos.z > t->mid.z) {
            next_tree = new_trees[LEFT_DOWN_NEAR];
        } else if(cn->pos.x > t->mid.x && cn->pos.y <= t->mid.y && cn->pos.z > t->mid.z) {
            next_tree = new_trees[RIGHT_DOWN_NEAR];
        } else if(cn->pos.x <= t->mid.x && cn->pos.y > t->mid.y && cn->pos.z <= t->mid.z) {
            next_tree = new_trees[LEFT_UP_FAR];
        } else if(cn->pos.x > t->mid.x && cn->pos.y > t->mid.y && cn->pos.z <= t->mid.z) {
            next_tree = new_trees[RIGHT_UP_FAR];
        } else if(cn->pos.x <= t->mid.x && cn->pos.y <= t->mid.y && cn->pos.z <= t->mid.z) {
            next_tree = new_trees[LEFT_DOWN_FAR];
        } else {
            next_tree = new_trees[RIGHT_DOWN_FAR];
        }

        if(next_tree->tail != NULL)
            next_tree->tail->next = cn;
        next_tree->tail = cn;
        if(next_tree->head == NULL)
            next_tree->head = cn;
        cn->next = NULL;

        next_tree->num_points++;
    }

    t->num_points = 0;
    t->head = NULL;
    t->tail = NULL;
    for(i=0; i<NUM_OCTANTS; i++) {
        t->octants[i] = new_trees[i];
        new_trees[i]->depth = t->depth + 1;
    }

    res = 0;
    if(t->depth+1+1 <= MAX_DEPTH) {
        for(i=0; i<NUM_OCTANTS; i++) {
            if(new_trees[i]->num_points > MAX_NUM_POINTS) {
                res |= octree_split(new_trees[i]);
            }
        }
    }

    return res;
}

#if 0

int octree_has(Octree * const t, const double x, const double y, const double z)
{
    int i;
    int found = 0;
    ListNode *cn;   /* current node pointer */

    cn = t->head;
    for(i=0; i<t->num_points; i++) {
        if(cn->pos.x == x && cn->pos.y == y && cn->pos.z == z) {
            found = 1;
            break;
        }

        cn = cn->next;
    }

    if(!found && (t->octants[LEFT_UP_NEAR] != NULL &&
                  t->octants[RIGHT_UP_NEAR] != NULL &&
                  t->octants[LEFT_DOWN_NEAR] != NULL &&
                  t->octants[RIGHT_DOWN_NEAR] != NULL &&
                  t->octants[LEFT_UP_FAR] != NULL &&
                  t->octants[RIGHT_UP_FAR] != NULL &&
                  t->octants[LEFT_DOWN_FAR] != NULL &&
                  t->octants[RIGHT_DOWN_FAR] != NULL)) {
        if(cn->pos.x <= t->mid.x && cn->pos.y > t->mid.y && cn->pos.z > t->mid.z) {
            found = octree_has(t->octants[LEFT_UP_NEAR], x, y, z);
        } else if(cn->pos.x > t->mid.x && cn->pos.y > t->mid.y && cn->pos.z > t->mid.z) {
            found = octree_has(t->octants[RIGHT_UP_NEAR], x, y, z);
        } else if(cn->pos.x <= t->mid.x && cn->pos.y <= t->mid.y && cn->pos.z > t->mid.z) {
            found = octree_has(t->octants[LEFT_DOWN_NEAR], x, y, z);
        } else if(cn->pos.x > t->mid.x && cn->pos.y <= t->mid.y && cn->pos.z > t->mid.z) {
            found = octree_has(t->octants[RIGHT_DOWN_NEAR], x, y, z);
        } else if(cn->pos.x <= t->mid.x && cn->pos.y > t->mid.y && cn->pos.z <= t->mid.z) {
            found = octree_has(t->octants[LEFT_UP_FAR], x, y, z);
        } else if(cn->pos.x > t->mid.x && cn->pos.y > t->mid.y && cn->pos.z <= t->mid.z) {
            found = octree_has(t->octants[RIGHT_UP_FAR], x, y, z);
        } else if(cn->pos.x <= t->mid.x && cn->pos.y <= t->mid.y && cn->pos.z <= t->mid.z) {
            found = octree_has(t->octants[LEFT_DOWN_FAR], x, y, z);
        } else {
            found = octree_has(t->octants[RIGHT_DOWN_FAR], x, y, z);
        }
    }

    return found;
}

#endif

// add some data to the tree
ListNode* octree_add(Octree * const t, const double x, const double y,
                     const double z, void * const data)
{
    if(t->octants[LEFT_UP_NEAR] == NULL ||
       t->octants[RIGHT_UP_NEAR] == NULL ||
       t->octants[LEFT_DOWN_NEAR] == NULL ||
       t->octants[RIGHT_DOWN_NEAR] == NULL ||
       t->octants[LEFT_UP_FAR] == NULL ||
       t->octants[RIGHT_UP_FAR] == NULL ||
       t->octants[LEFT_DOWN_FAR] == NULL ||
       t->octants[RIGHT_DOWN_FAR] == NULL) {
        if(t->depth >= MAX_DEPTH || t->num_points < MAX_NUM_POINTS) {
            // then add data point to the current node
            // if(octree_has(t, x, y, z)) {
                // fprintf(stderr, "attempting to add the same point. failed!\n");
                // return NULL;
            // }

            // fprintf(stderr,"adding point (%f, %f, %f)\n",x,y,z);

            ListNode *new_point = (ListNode*) malloc(sizeof(ListNode));
            new_point->pos.x = x;
            new_point->pos.y = y;
            new_point->pos.z = z;
            new_point->data = data;
            new_point->next = NULL;

            if(t->tail != NULL)
                t->tail->next = new_point;
            t->tail = new_point;
            if(t->head == NULL)
                t->head = new_point;

            t->num_points++;
            return new_point;
        } else {
            // split the current node
            octree_split(t);
        }
    }

    // traverse the tree
    if(x <= t->mid.x && y > t->mid.y && z > t->mid.z) {
        return octree_add(t->octants[LEFT_UP_NEAR], x, y, z, data);
    } else if(x > t->mid.x && y > t->mid.y && z > t->mid.z) {
        return octree_add(t->octants[RIGHT_UP_NEAR], x, y, z, data);
    } else if(x <= t->mid.x && y <= t->mid.y && z > t->mid.z) {
        return octree_add(t->octants[LEFT_DOWN_NEAR], x, y, z, data);
    } else if(x > t->mid.x && y <= t->mid.y && z > t->mid.z) {
        return octree_add(t->octants[RIGHT_DOWN_NEAR], x, y, z, data);
    } else if(x <= t->mid.x && y > t->mid.y && z <= t->mid.z) {
        return octree_add(t->octants[LEFT_UP_FAR], x, y, z, data);
    } else if(x > t->mid.x && y > t->mid.y && z <= t->mid.z) {
        return octree_add(t->octants[RIGHT_UP_FAR], x, y, z, data);
    } else if(x <= t->mid.x && y <= t->mid.y && z <= t->mid.z) {
        return octree_add(t->octants[LEFT_DOWN_FAR], x, y, z, data);
    } else {
        return octree_add(t->octants[RIGHT_DOWN_FAR], x, y, z, data);
    }
}

int delete_octree(Octree * const tree)
{
    int i;

    if(tree->num_points > 0) {
        ListNode *cn;
        for(i=0; i<tree->num_points; i++) {
            cn = tree->head;
            tree->head = tree->head->next;
            free(cn);
        }
    }

    for(i=0; i<NUM_OCTANTS; i++) {
        if(tree->octants[i] != NULL) {
            delete_octree(tree->octants[i]);
        }
    }

    free(tree);

    return 0;
}
