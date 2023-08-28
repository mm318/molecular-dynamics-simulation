#ifndef _QUADTREE_H_
#define _QUADTREE_H_

#include "my_rand.h"


#define NUM_QUADRANTS   4

typedef enum {
    LEFT_UP = 0,   // (-x, +y, +z)
    RIGHT_UP,      // (+x, +y, +z)
    LEFT_DOWN,     // (-x, -y, +z)
    RIGHT_DOWN,    // (+x, -y, +z)
} Quadrant;

typedef struct list_node ListNode;
struct list_node {
    double y_pos;
    double x_pos;
    void *data;
    ListNode *next;
};

typedef struct qtree QTree;
struct qtree {
    double y_min;
    double y_max;
    double y_mid;

    double x_min;
    double x_max;
    double x_mid;

    union {
        struct {
            QTree *upper_left;
            QTree *upper_right;
            QTree *lower_left;
            QTree *lower_right;
        };
        QTree *quadrants[NUM_QUADRANTS];
    };

    int num_points;
    ListNode *head;
    ListNode *tail;

    // debug
    int depth;
};


QTree* new_quadtree(const double y_min, const double y_max,
                    const double x_min, const double x_max);

int quadtree_has(QTree * const tree, const double y, const double x);

ListNode* quadtree_add(QTree * const tree, const double y, const double x,
                       void * const data);

int delete_quadtree(QTree * const tree);


// used for debug
// void quadtree_dump(QTree * const tree, Frame * const frame, char * const filename,
// Seed * const s, int level);


#endif
