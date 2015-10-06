#ifndef __CELLINFO_H__
#define __CELLINFO_H__

#include "mesh.h"
#include "cell.h"

/*
 * -----------------------------------------------------------------
 *
 *                        CONSTANTS 
 *
 * -----------------------------------------------------------------
 */

#define NB_SCORE_MAX 5          /**< maximum number of score in a cell */

#define NB_HIT_FACES 6          /**< number of distinguished faces that can be crossed over by rays*/

#define NB_CELL_BLOCKS 4        /**< NB_CELL_BLOCKS^3 cell subdivisions inside a cell (for disp_score)*/

#define NB_CELL_BLOCKS3 64      /**< NB_CELL_BLOCKS^3 as a constant to avoid computations */
#define NB_R2M_SCORE 5

enum { SCORE = 0, HIT, LENGTH, DISP, NB };

enum { NORTH_FACE = 0, SOUTH_FACE, EAST_FACE, WEST_FACE, UP_FACE, DOWN_FACE
};

/*
 * -----------------------------------------------------------------
 *
 *                         DATA STRUCTURES
 *
 * -----------------------------------------------------------------
 */

/** \brief trivial information on a given ray in the given cell */
struct cell_info_item_t {

    double P_length;               /**< P part ray length */

    double S_length;               /**< S part ray length */

    struct coord_geo_t *in,        /**< input ray point */

    *out;                          /**< output ray point */

    long int rayid;                /**< unique ray ID */
};

typedef struct cell_info_item_t cell_info_item_t;

/** \brief definition of storage type for small blocks
 *
 * type for blocknum_t must match the NB_CELL_BLOCKS :
 * e.g. unsigned char let code 256 possible block ids. 
 * Thus, NB_CELL_BLOCKS must not exceed 6, since 6^3 = 216, 7^3 = 343 which would overflow
 */
typedef unsigned char blocknum_t;

/** \brief score information summary for one given cell */
struct cell_info_t {

    int nitems;                    /**< number of items for this cell */

    struct cell_info_item_t *item; /**< each ray brings some information when it crosses the cell. 
				        For each ray, an info. item is created and added to the 
					item list. */

    int *faces_hit;                /**< a 6-int zone: DOWN,UP,WEST,EAST,NORTH,SOUTH */

    int nblocks;                   /**< number of blocks hit */

    blocknum_t *block_hit;         /**< a NB_CELL_BLOCKS^3 elements zone (contains the block number)*/

    /* scores */

    float *score;                  /**< score array */
};

typedef struct cell_info_t cell_info_t;

/*
 * -----------------------------------------------------------------
 *
 *                               METHODS  
 *
 * -----------------------------------------------------------------
 */

/*
 * -----------------------------------------------------------
 *
 *                       ACCESS METHODS
 *
 * -----------------------------------------------------------
 */

void mesh_cellinfo_free(struct cell_info_t *c);

struct cell_info_t *mesh_cellinfo_onecell_alloc();

struct cell_info_t ****mesh_cellinfo_alloc(const struct mesh_t *m);

void
mesh_cellinfo_length_feed(struct cell_info_t *c,
                          const struct coord_geo_t *in,
                          const struct coord_geo_t *out,
                          const double P_length,
                          const double S_length, const long int rid);

int
mesh_cellinfo_facehit_get(const struct mesh_t *mesh,
                          const struct coord_z3_t *c0,
                          const struct coord_z3_t *c1);

void
mesh_cellinfo_facehit_feed(const struct mesh_t *mesh,
                           struct cell_info_t ****c,
                           const struct coord_z3_t *c0,
                           const struct coord_z3_t *c1);

void mesh_cellinfo_singlefacehit_feed(const struct mesh_t *mesh,
                                      struct cell_info_t ****c,
                                      const struct coord_z3_t *c0,
                                      const struct coord_z3_t *c1);
void mesh_cellinfo_block_feed(struct cell_info_t *ci, blocknum_t block_id);

int mesh_cellinfo_rayids_increment(struct cell_info_t ****c,
                                   const struct mesh_t *m, const int incr);

int mesh_cellinfo_point2cell(const struct mesh_t *mesh,
                             const double x0, const double y0,
                             const double z0, struct coord_z3_t *cell_id);

int mesh_cellinfo_which_layer(const struct mesh_t *mesh,
                              const double z, int *layer);

/* -----------------------------------------------------------
 *
 *                       DUMP METHODS
 *
 * -----------------------------------------------------------
 */
void mesh_cellinfo_facehit_dump(FILE * fd, struct cell_info_t *cell);

void
mesh_cellinfo_dump(FILE * fd, struct cell_info_t ****c, struct mesh_t *m);

int mesh_cellinfo_write_sco(char *filename,
                            struct cell_info_t ****c, struct mesh_t *m);

long int mesh_cellinfo_get_max_rayid(struct cell_info_t ****c,
                                     struct mesh_t *m);

#endif
