#include "geometry.h"
#include <stdlib.h>
#include <math.h>
#include <stdio.h>


void ray_box_isct(Vector a, Vector b, Box box, Vector* min, Vector* max);

/** Accumulates the minimum components of both vectors in v1 */
void min_components(Vector* /*out*/ v1, Vector* v2){
    if (v2->X < v1->X) v1->X = v2->X;
    if (v2->Y < v1->Y) v1->Y = v2->Y;
    if (v2->Z < v1->Z) v1->Z = v2->Z;
}

/** Accumulates the maximum components of both vectors in v1 */
void max_components(Vector* /*out*/ v1, Vector* v2) {
    if (v2->X > v1->X) v1->X = v2->X;
    if (v2->Y > v1->Y) v1->Y = v2->Y;
    if (v2->Z > v1->Z) v1->Z = v2->Z;
}


bool clip_vector(Vector a, Vector b, Box box, Vector* new);
void clip_component(Vector* a, Vector b, float limit, int cpt);

bool ray_box_intersection(Vector min, Vector max, Ray* ray){
    Vector dir = ray_get_direction(ray);
    Vector idir;

    idir.X = 1.f/dir.X;
    idir.Y = 1.f/dir.Y;
    idir.Z = 1.f/dir.Z;

    Vector orig = ray_get_origin(ray);
    float tmin, tmax;


    float txmin = (min.X - orig.X) * idir.X;
    float txmax = (max.X - orig.X) * idir.X;

    tmin = min(txmin, txmax);
    tmax = max(txmin, txmax);

    float tymin = (min.Y - orig.Y) * idir.Y;
    float tymax = (max.Y - orig.Y) * idir.Y;

    tmin = max(tmin, min(tymin, tymax));
    tmax = min(tmax, max(tymin, tymax));

    float tzmin = (min.Z - orig.Z) * idir.Z;
    float tzmax = (max.Z - orig.Z) * idir.Z;

    tmin = max(tmin, min(tzmin, tzmax));
    tmax = min(tmax, max(tzmin, tzmax));

    return tmin <= tmax;
}

bool point_in_box(Vector p, Box b){
    if (p.X < b.min.X || p.X > b.max.X) return false;
    if (p.Y < b.min.Y || p.Y > b.max.Y) return false;
    if (p.Z < b.min.Z || p.Z > b.max.Z) return false;
    return true;
}


bool clip_triangle1(Triangle* t, Box b, Vector* min, Vector* max){
    Vector* vertices = malloc(sizeof(Vector) * 10);
    Vector* accepted = malloc(sizeof(Vector) * 20);

    size_t nvertex = 3;
    size_t n_accepted = 0;
    size_t new;

    triangle_get_vertices(t, vertices, vertices + 1, vertices + 2);

    Vector v;

    vector_min(max);
    vector_max(min);

    while (nvertex > 0){
        v = vertices[--nvertex];

        if (point_in_box(v, b))
            accepted[n_accepted++] = v;
        else {
            new = 0;
            for (size_t i = 0; i < nvertex; ++i) {
                if (clip_vector(v, vertices[i], b, &vertices[nvertex + new])) new++;
            }
            for (size_t i = 0; i < n_accepted; ++i)
                if (clip_vector(v, accepted[i], b, &vertices[nvertex + new])) new++;
            nvertex += new;
        }
    }

    for (int j = 0; j < n_accepted; ++j) {
        min_components(min, &accepted[j]);
        max_components(max, &accepted[j]);
    }

    free(vertices);
    free(accepted);

    return (min->X <= max->X && min->Y <= max->Y && min->Z <= max->Z);
}


bool clip_triangle(Triangle* t, Box box, Vector* min, Vector* max){
    vector_min(max);
    vector_max(min);

    Vector a, b, c;

    triangle_get_vertices(t, &a, &b, &c);

    min_components(min, &a);
    max_components(max, &a);
    min_components(min, &b);
    max_components(max, &b);
    min_components(min, &c);
    max_components(max, &c);


    if (box.max.X < max->X) max->X = box.max.X;
    if (box.min.X > min->X) min->X = box.min.X;

    if (box.max.Y < max->Y) max->Y = box.max.Y;
    if (box.min.Y > min->Y) min->Y = box.min.Y;

    if (box.max.Z < max->Z) max->Z = box.max.Z;
    if (box.min.Z > min->Z) min->Z = box.min.Z;

    return (min->X <= max->X && min->Y <= max->Y && min->Z <= max->Z);
}

void ray_box_isct(Vector a, Vector b, Box box, Vector* min, Vector* max){
    Vector ab;

    if (ray_plane_intersection(a, b, box.min.X, X, &ab) && point_in_box(ab, box)){
        min_components(min, &ab);
        max_components(max, &ab);
    }
    if (ray_plane_intersection(a, b, box.max.X, X, &ab) && point_in_box(ab, box)){
        min_components(min, &ab);
        max_components(max, &ab);
    }

    if (ray_plane_intersection(a, b, box.min.Y, Y, &ab) && point_in_box(ab, box)){
        min_components(min, &ab);
        max_components(max, &ab);
    }
    if (ray_plane_intersection(a, b, box.max.Y, Y, &ab) && point_in_box(ab, box)){
        min_components(min, &ab);
        max_components(max, &ab);
    }

    if (ray_plane_intersection(a, b, box.min.Z, Z, &ab) && point_in_box(ab, box)){
        min_components(min, &ab);
        max_components(max, &ab);
    }
    if (ray_plane_intersection(a, b, box.max.Z, Z, &ab) && point_in_box(ab, box)){
        min_components(min, &ab);
        max_components(max, &ab);
    }
}


/** Clips the line defined by a - b towards b. Returns true if the line is clipped, false otherwise.*/
bool clip_vector(Vector a, Vector b, Box box, Vector* new){
    if (vector_equal(a, b)) return false;

    *new = a;

    if (new->X > box.max.X && b.X <= box.max.X) clip_component(new, b, box.max.X, 0);
    if (new->X < box.min.X && b.X >= box.min.X) clip_component(new, b, box.min.X, 0);

    if (new->Y > box.max.Y && b.Y <= box.max.Y) clip_component(new, b, box.max.Y, 1);
    if (new->Y < box.min.Y && b.Y >= box.min.Y) clip_component(new, b, box.min.Y, 1);

    if (new->Z > box.max.Z && b.Z <= box.max.Z) clip_component(new, b, box.max.Z, 2);
    if (new->Z < box.min.Z && b.Z >= box.min.Z) clip_component(new, b, box.min.Z, 2);

    if (vector_equal(*new, a)) return false;

    return true;
}

void clip_component(Vector* a, Vector b, float limit, int cpt){
    float av = cpt == 0? a->X : cpt == 1? a->Y : a->Z;
    float bv = cpt == 0? b.X : cpt == 1? b.Y : b.Z;
    float ratio = (bv - limit) / (bv - av);

    a->X = b.X + (a->X - b.X) * ratio;
    a->Y = b.Y + (a->Y - b.Y) * ratio;
    a->Z = b.Z + (a->Z - b.Z) * ratio;
}

bool vector_equal(Vector a, Vector b){
    return a.X == b.X && a.Y == b.Y && a.Z == b.Z;
}

bool box_equal(Box a, Box b){
    return vector_equal(a.min, b.min) && vector_equal(a.max, b.max);
}

Box triangle_bounding_box(Triangle* t){
    Vector a, b, c;
    Box box;
    vector_min(&box.max);
    vector_max(&box.min);

    triangle_get_vertices(t, &a, &b, &c);

    min_components(&box.min, &a);
    min_components(&box.min, &b);
    min_components(&box.min, &c);

    max_components(&box.max, &a);
    max_components(&box.max, &b);
    max_components(&box.max, &c);

    return box;
}


void print_vector(Vector v, const char end[]){
    printf("(%f, %f, %f)%s", v.X, v.Y, v.Z, end);
}

bool split_box(Box* v, float pos, Axis a, Box* /*out*/ left, Box* /*out*/ right){
    *left = *v;
    *right = *v;

    switch (a){
        case X:
            if (pos > v->max.X || pos < v->min.X) return false;
            left->max.X = pos;
            right->min.X = pos;
            break;
        case Y:
            if (pos > v->max.Y || pos < v->min.Y) return false;
            left->max.Y = pos;
            right->min.Y = pos;
            break;
        case Z:
            if (pos > v->max.Z || pos < v->min.Z) return false;
            left->max.Z = pos;
            right->min.Z = pos;
            break;
    }

    return true;
}

float surface_area(Box b) {
    float w,h,l;

    w = b.max.X - b.min.X;
    h = b.max.Y - b.min.Y;
    l = b.max.Z - b.min.Z;

    return 2*(w*h + w*l + h*l);
}


int clip_triangle_to_plane(Triangle* t, float pos, Axis axis){
    Vector a, b, c, min, max;
    triangle_get_vertices(t, &a, &b, &c);
    vector_min(&max);
    vector_max(&min);

    min_components(&min, &a);
    min_components(&min, &b);
    min_components(&min, &c);

    max_components(&max, &a);
    max_components(&max, &b);
    max_components(&max, &c);

    switch (axis){
        case X:
            if (min.X == max.X && min.X == pos) return 1;
            if (max.X <= pos) return 0;
            if (min.X >= pos) return 2;
            break;
        case Y:
            if (min.Y == max.Y && min.Y == pos) return 1;
            if (max.Y <= pos) return 0;
            if (min.Y >= pos) return 2;
            break;
        case Z:
            if (min.Z == max.Z && min.Z == pos) return 1;
            if (max.Z <= pos) return 0;
            if (min.Z >= pos) return 2;
            break;
    }
    return 3;
}


bool ray_plane_intersection(Vector a, Vector b, float pos, Axis axis, Vector* isct){
    Vector l = vector_substracted_v(b, a);
    vector_normalize(&l);

    Vector p, n;
    p.X = p.Y = p.Z = 0.f;
    n.X = n.Y = n.Z = 0.f;

    switch (axis){
        case X:
            p.X = pos;
            n.X = 1.f;
            break;
        case Y:
            p.Y = pos;
            n.Y = 1.f;
            break;
        case Z:
            p.Z = pos;
            n.Z = 1.f;
            break;
    }

    float l_n = vector_dot(l, n);

    if (l_n > -EPSILON && l_n < EPSILON)
        return false;

    float d = vector_dot(vector_substracted_v(p, a), n) / l_n;

    if (d < 0)
        return false;

    Vector dist = vector_multiplied_f(l, d);

    *isct = vector_added_v(a, dist);
    return true;
}