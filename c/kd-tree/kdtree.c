#include "kdtree.h"
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <ranlib.h>

/* UTILITIES */
struct kd_node* kdnode_new();
void kdnode_free(struct kd_node* node);

void SAH(Box V, size_t NL, size_t NR, Plane* p);

typedef enum {END, PLANAR, START} Type;

int TOT = 0;
int leaf_nodes = 0;
int nodes = 0;

typedef struct event{
    Type type;
    Axis axis;
    float position;
    Triangle* triangle;
} Event;

int cmpE(const void* pa, const void* pb);
size_t generate_events(Triangle* triangle, Box box, Axis axis, Event* e1, Event* e2);

void finish_node(struct kd_node* node, Triangle** data, size_t t_count){
    node->content = malloc(sizeof(Triangle *) * t_count);
    for (int i = 0; i < t_count; ++i) {
        node->content[i] = data[i];
    }
    node->content_len = t_count;

    leaf_nodes++;
    TOT += t_count;
}

void fill_node(struct kd_node* node, Triangle** triangles, size_t t_count){

    Event* events = malloc(sizeof(Event) * t_count * 6);
    size_t e_count = 0;
    for (int i = 0; i < t_count; ++i) {
        e_count += generate_events(triangles[i], node->box, X, events + e_count, events + e_count + 1);
        e_count += generate_events(triangles[i], node->box, Y, events + e_count, events + e_count + 1);
        e_count += generate_events(triangles[i], node->box, Z, events + e_count, events + e_count + 1);
    }

    qsort(events, e_count, sizeof(Event), cmpE);

    size_t NL[] = {0, 0, 0};
    size_t NP[] = {0, 0, 0};
    size_t NR[] = {t_count, t_count, t_count};

    Plane p, P;
    P.cost = INFINITY;

    size_t end, planar, start;

    for (size_t j = 0; j < e_count; ++j) {
        end = planar = start = 0;
        p.position = events[j].position;
        p.axis = events[j].axis;

        while (j < e_count && events[j].axis == p.axis && events[j].position == p.position && events[j].type == END){
            end++;
            j++;
        }

        while (j < e_count && events[j].axis == p.axis && events[j].position == p.position && events[j].type == PLANAR){
            planar++;
            j++;
        }

        while (j < e_count && events[j].axis == p.axis && events[j].position == p.position && events[j].type == START){
            start++;
            j++;
        }

        NP[p.axis] = planar;
        NR[p.axis] -= (end + planar);

        SAH(node->box, NL[p.axis], NR[p.axis] + NP[p.axis], &p);

        if (p.cost < P.cost)
            P = p;

        NP[p.axis] = 0;
        NL[p.axis] += (start + planar);
    }

    free(events);

    node->left = kdnode_new();
    node->right = kdnode_new();
    split_box(&node->box, P.position, P.axis, &node->left->box, &node->right->box);

    if (P.cost > KI*t_count || box_equal(node->left->box, node->box) || box_equal(node->right->box, node->box)) {
        kdnode_free(node->left);
        kdnode_free(node->right);
        node->left = node->right = NULL;
        return finish_node(node, triangles, t_count);
    }


    Triangle** t_left = malloc(sizeof(Triangle*) * t_count);
    Triangle** t_right = malloc(sizeof(Triangle*) * t_count);

    size_t tc_left = 0;
    size_t tc_right = 0;


    for (int i = 0; i < t_count; ++i) {
        int isect = clip_triangle_to_plane(triangles[i], P.position, P.axis);
        if (isect == 0 || isect == 3)
            t_left[tc_left++] = triangles[i];
        if (isect >= 1)
            t_right[tc_right++] = triangles[i];
    }

    fill_node(node->left, t_left, tc_left);
    fill_node(node->right, t_right, tc_right);

    free(t_left);
    free(t_right);
}


KDTree* KDTree_new(Triangle** triangles, size_t t_count){
    struct kd_node* root = kdnode_new();

    for (int i = 0; i < t_count; ++i) {
        Vector a, b, c;
        triangle_get_vertices(triangles[i], &a, &b, &c);

        max_components(&root->box.max, &a);
        max_components(&root->box.max, &b);
        max_components(&root->box.max, &c);

        min_components(&root->box.min, &a);
        min_components(&root->box.min, &b);
        min_components(&root->box.min, &c);
    }

    fill_node(root, triangles, t_count);

    return root;
}


/** creates a new kd_node */
struct kd_node* kdnode_new(){
    nodes++;

    struct kd_node* node = malloc(sizeof(struct kd_node));
    node->left = node->right = NULL;
    node->content = NULL;
    node->content_len = 0;

    vector_min(&node->box.max);
    vector_max(&node->box.min);

    return node;
}

/** Recursive kdnode free */
void kdnode_free(struct kd_node* node){
    if (node == NULL) return;

    nodes--;

    kdnode_free(node->left);
    kdnode_free(node->right);

    if (node->content != NULL){
        free(node->content);
    }
    free(node);
}


/** Destroys a KD Tree, freeing its memory usage */
void KDTree_destroy(KDTree* tree){
    printf("TOTAL TRIS : %d\n", TOT);
    printf("TOTAL NODES: %d\n", nodes);
    printf("LEAF NODES : %d  (~TRIS per node: %f)\n", leaf_nodes, ((float)TOT)/leaf_nodes);
    kdnode_free(tree);
}

int cmpE(const void* pa, const void* pb){
    Event a = *((Event *) pa);
    Event b = *((Event *) pb);
    if (a.position < b.position ||
            (a.position == b.position && a.axis < b.axis) ||
            (a.position == b.position && a.axis == b.axis && a.type < b.type))
        return -1;
    return 1;
}


void SAH(Box V, size_t NL, size_t NR, Plane *p){
    Box L, R;
    split_box(&V, p->position, p->axis, &L, &R);
    float prL = surface_area(L) / surface_area(V);
    float prR = surface_area(R) / surface_area(V);

    p->cost = KT + KI*(prL * NL + prR * NR);
}

size_t generate_events(Triangle* triangle, Box box, Axis axis, Event* e1, Event* e2){

    Box t = triangle_bounding_box(triangle);

    e1->axis = e2->axis = axis;

    switch (axis){
        case X:
            e1->position = max(t.min.X, box.min.X);
            e2->position = min(t.max.X, box.max.X);
            break;
        case Y:
            e1->position = max(t.min.Y, box.min.Y);
            e2->position = min(t.max.Y, box.max.Y);
            break;
        case Z:
            e1->position = max(t.min.Z, box.min.Z);
            e2->position = min(t.max.Z, box.max.Z);
            break;
    }


    if (e1->position == e2->position){
        e1->type = PLANAR;
        return 1;
    }
    else {
        e1->type = START;
        e2->type = END;
        return 2;
    }
}