#include <stdio.h>
#include <errno.h>

#include "mesh.h"
#include "layer.h"
#include "cell.h"

#ifndef __MESH_IMPORT_H__
#define __MESH_IMPORT_H__

struct mesh_offset_t {
    int lat;
    int lon;
    int z;
};

struct mesh_offset_t **compute_mesh_offset(struct mesh_t *m1,
                                           struct mesh_t *m2);

struct mesh_offset_t *compute_mesh_offset_in_layer(struct mesh_t *m1,
                                                   struct mesh_t *m2,
                                                   int l);

int compare_rayid(const void *a, const void *b);

/** \brief Import in mesh sco formated files generated by ray2mesh.
 *   
 *    Import in a mesh structure several sco formated files generated by ray2mesh
 *    the data format is described below : 
 *
 *    Example:
 *
\verbatim   
5 Score Hit Length Disp nbrays
[0,9,0] 0.01 0.17 0.07 0.05 1
[0,10,0] 0.01 0.17 0.07 0.05 2
[0,11,0] 0.02 0.17 0.09 0.06 9
...
\endverbatim
*/
int import2mesh_sco_file(struct mesh_t *mesh,

                                        /**< existing mesh structure to import data */

                         char *filename,/**< sco files names to import (comma separated list) */
                         int use_log_scale,

                                        /**< if the score needs to be log scaled */
                         struct mesh_offset_t **offset);

                                                /**< use offset to translate the data */

/** \brief Import in mesh r2m formated file generated by ray2mesh 
 *   
 *    Import in a mesh structure several r2m formated files generated by ray2mesh
 *    the data format is described below : 
 *
 *    Example:
 *
\verbatim   
# domain 0/2
1248
0 0 9
7 0 0 0 0 0
3 25 41 57
1
3522 0.538277 -0.230541 10.000000 0.540587 -0.230624 38.384984 32.000943 0.000000

0 0 10
2 0 0 0 0 0
3 29 45 61
2
5254 0.546812 -0.164689 10.000000 0.549168 -0.163455 39.727551 34.001091 0.000000
5255 0.546812 -0.164689 10.000000 0.549207 -0.163674 39.849062 34.001066 0.000000
...
\endverbatim

 */
int import2mesh_r2m_file(struct mesh_t *mesh,

                                        /**< existing mesh structure where to import data */

                         char *filename,/**< r2m files names to import (comma separated list) */
                         struct mesh_offset_t **offset);

                                                /**< use offset to translate the data */

int import2mesh_irm_file(struct mesh_t *mesh,
                         char *filename, struct mesh_offset_t **offset);

#endif
