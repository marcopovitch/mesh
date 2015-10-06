#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "const.h"

#ifndef __POINT3D_H__
#define __POINT3D_H__

#ifndef _COORD_3D_H_
#define _COORD_3D_H_

/** \brief point definition in a 3D space */
struct point3d_t {

    float lat;          /**< latitude in degree */

    float lon;          /**< longitude in degree */

    float prf;          /**< depth */
};
#endif

/** \brief structure to keep/allocate point3d */
struct point3d_tab_t {

    struct point3d_t **tab;     /**< array */

    int nbpoint;                /**< nb of points in array */

    int blockminsize;           /**< size of the allocated block for point3d */
};

struct point3d_tab_t *add_point3d_to_tab(struct point3d_tab_t *tabpoint,
                                         struct point3d_t *point);
int create_point3d(struct point3d_tab_t *tab, float lat, float lon,
                   float prf);
void destroy_point3d(struct point3d_t *point3d);

void dump_point3d(char *s, struct point3d_t *p);
#endif
