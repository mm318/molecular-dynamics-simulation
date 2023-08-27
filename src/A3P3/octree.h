#ifndef _OCTREE_H_
#define _OCTREE_H_

#include "my_rand.h"


#define NUM_OCTANTS 8

// in opengl coordinate system
typedef enum {
    LEFT_UP_NEAR = 0,   // (-x, +y, +z)
    RIGHT_UP_NEAR,      // (+x, +y, +z)
    LEFT_DOWN_NEAR,     // (-x, -y, +z)
    RIGHT_DOWN_NEAR,    // (+x, -y, +z)
    LEFT_UP_FAR,        // (-x, +y, -z)
    RIGHT_UP_FAR,       // (+x, +y, -z)
    LEFT_DOWN_FAR,      // (-x, -y, -z)
    RIGHT_DOWN_FAR,     // (+x, -y, -z)
} Octant;

typedef struct {
    double x;
    double y;
    double z;
} Coord;

typedef struct list_node ListNode;
struct list_node {
    Coord pos;
    void *data;
    ListNode *next;
};

typedef struct octree Octree;
struct octree {
    Coord min;
    Coord mid;
    Coord max;

    Octree *octants[NUM_OCTANTS];

    int num_points;
    ListNode *head;
    ListNode *tail;

    // debug
    int depth;
};


Octree* new_octree(const double x_min, const double x_max, const double y_min,
                   const double y_max, const double z_min, const double z_max);

int octree_has(Octree * const tree, const double x, const double y, const double z);

ListNode* octree_add(Octree * const tree, const double x, const double y,
                     const double z, void * const data);

int delete_octree(Octree * const tree);


#endif
