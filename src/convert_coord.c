#include <assert.h>
#include <stdlib.h>

#include "convert_coord.h"
#include "const.h"

struct coord_geo_t *point3d2geo(struct point3d_t *p)
{
    struct coord_geo_t *g =
        (struct coord_geo_t *) malloc(sizeof(struct coord_geo_t));
    assert(g);
    g->lat = p->lat;
    g->lon = p->lon;
    g->prof = p->prf;
    return (g);
}

/**
 * coordinates transformation : point3d (georaphic) (angles are in radians)
 * coordinates to cartesian coordinates                           
 */
struct coord_cart_t *point3d2cart(const struct point3d_t *point3d)
{
    struct coord_cart_t *coord_cart;

    double r, cos_lat, sin_lat, cos_lon, sin_lon;

    coord_cart =
        (struct coord_cart_t *) malloc(sizeof(struct coord_cart_t));
    assert(coord_cart);

    r = RE - point3d->prf;
    cos_lat = cos(point3d->lat * TO_RAD);
    sin_lat = sin(point3d->lat * TO_RAD);
    cos_lon = cos(point3d->lon * TO_RAD);
    sin_lon = sin(point3d->lon * TO_RAD);

    coord_cart->x = r * cos_lat * cos_lon;
    coord_cart->y = r * cos_lat * sin_lon;
    coord_cart->z = r * sin_lat;

    return (coord_cart);
}
