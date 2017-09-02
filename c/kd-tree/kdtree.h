#ifndef IIC2133_KDTREE_H
#define IIC2133_KDTREE_H

#include "geometry.h"
#include <stdlib.h>


#define KI 20.f
#define KT 15.f

struct kd_node{
    /** Left subtree */
    struct kd_node* left;

    /** Right subtree */
    struct kd_node* right;

    /** Represents the volume enclosed by the node */
    Box box;

    /** Possible content */
    Triangle** content;

    /** number of triangles in content */
    size_t content_len;
};


typedef struct kd_node KDTree;

/** Creates a new KD Tree over the given data*/
KDTree* KDTree_new(Triangle** triangles, size_t t_count);

/** Destroys a KD Tree, freeing its memory usage */
void KDTree_destroy(KDTree*);




#endif //IIC2133_KDTREE_H
