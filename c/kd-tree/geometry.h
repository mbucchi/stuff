#ifndef LIB_GEOMETRY_W
#define LIB_GEOMETRY_W

/*#########################################################################*/
/*                                Geometría                                */
/*                                                                         */
/* Éste módulo encapsula los componentes y operaciones de la               */
/* geometría del problema                                                  */
/*#########################################################################*/

#include "vector.h"
#include <stdbool.h>
#include <math.h>

#define EPSILON 0.2

#define min(x, y) (x < y? x : y)
#define max(x, y) (x > y? x : y)

#define vector_min(vecptr) (vecptr)->X = (vecptr)->Y = (vecptr)->Z = -INFINITY
#define vector_max(vecptr) (vecptr)->X = (vecptr)->Y = (vecptr)->Z = INFINITY

/*############################  Triángulos  ###############################*/

struct triangle;
/** Representa un triangulo en R³ */
typedef struct triangle Triangle;

/** Guarda en los punteros los vertices de un triángulo */
void triangle_get_vertices(Triangle* tri, Vector* v1, Vector* v2, Vector* v3);

/*##############################  Rayos  ##################################*/

struct ray;
/** Representa un rayo, con un origen y una dirección */
typedef struct ray Ray;

/** Entrega el origen del rayo */
Vector    ray_get_origin              (Ray* ray);

/** Entrega la dirección del rayo */
Vector    ray_get_direction           (Ray* ray);

/** Intenta intersectar el rayo con el triángulo.
    En caso de éxito y que esté más cerca que el triángulo anterior que
    haya intersectado, se almacenarán los datos de la intersección         */
void      ray_intersect               (Ray* ray, Triangle* tri);

/** Obtiene el objeto más cercano que ha intersectado con el rayo.
    En caso de no haber intersectado con nada, retorna NULL                */
Triangle* ray_get_intersected_object  (Ray* ray);

/** Obtiene el punto donde se produjo la intersección */
Vector    ray_get_intersection_point  (Ray* ray);


/* NEW */

void min_components(Vector* /*out*/ v1, Vector* v2);
void max_components(Vector* /*out*/ v1, Vector* v2);
bool vector_equal(Vector a, Vector b);

typedef struct box{
    Vector min;
    Vector max;
} Box;

typedef enum {X, Y, Z} Axis;


typedef struct plane{
    float position;
    Axis axis;
    float cost;
} Plane;

/** Returns true if the given ray intersects that boundaries given by min and max*/
bool ray_box_intersection(Vector min, Vector max, Ray* ray);

/** Returns true if the given point lies within the AABB defined by min and max */
bool point_in_box(Vector p, Box b);

/** Returns true if part of the triangle lies within the box, also returns the max and min components of
 * that triangle within the box */
bool clip_triangle(Triangle* t, Box b, Vector* min, Vector* max);

void print_vector(Vector v, const char end[]);

Box triangle_bounding_box(Triangle* t);

bool split_box(Box* V, float pos, Axis a, Box* /*out*/ left, Box* /*out*/ right);

float surface_area(Box V);

int clip_triangle_to_plane(Triangle* t, float pos, Axis a);

bool ray_plane_intersection(Vector a, Vector b, float pos, Axis axis, Vector* isct);

bool box_equal(Box a, Box b);

#endif /* end of include guard: LIB_GEOMETRY_W */
