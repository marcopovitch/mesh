#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <assert.h>

#include "cellinfo.h"
#include "convert_coord.h"
#include "cell.h"
#include "mesh.h"
#include "layer.h"

char *R2M_SCORE_NAME[] = { "Score", "Hit", "Length", "Disp", "nbrays" };

/*
 * -----------------------------------------------------------
 *
 *                       ACCESS METHODS
 *
 * -----------------------------------------------------------
 */

/** \brief  Allocation of one cell.
 *
 *  Allocation of one cell that will be a part of the light mesh.
 **/
struct cell_info_t *mesh_cellinfo_onecell_alloc()
{
    struct cell_info_t *onecell;

    onecell = (struct cell_info_t *) calloc(1, sizeof(struct cell_info_t));
    assert(onecell);
    onecell->faces_hit = (int *) calloc(NB_HIT_FACES, sizeof(int));
    assert(onecell->faces_hit);

    onecell->score = (float *) calloc(NB_R2M_SCORE, sizeof(float));
    assert(onecell->score);

    return (onecell);
}

/** \brief free all the info stored in this structure 
 **/
void mesh_cellinfo_free(struct cell_info_t *c)
{

/**<  cell to free */
    int i;

    for (i = 0; i < c->nitems; i++) {
        free(c->item[i].in);
        free(c->item[i].out);
    }

    free(c->item);

    if (c->nblocks) {
        free(c->block_hit);
    }

    if (c->faces_hit) {
        free(c->faces_hit);
    }

    if (c->score) {
        free(c->score);
    }

    free(c);
    c = NULL;
}

/**
 * \brief create the light mesh structure from mesh information
 *
 * m. c array is ordered layer/lat/lon wise.
 *
 \verbatim
       +----- nlayers ------------+
  c -> |  |  |  |  |  |  |  |  |  |   layer
       +--+--+--+--+--+--+--+--+--+
        |
        |   +--+-lat_unit--+------+
        +-->|  |  |  |  |  |          lat
            +--+--+--+--+--+------+
             |
             |   +--+-lon_unit----+
             +-->|  |  |              lon
                 +--+--+----------+
                  | 
                  |  +-------------+
                  +->| cell_info_t |
                     +-------------+
 \endverbatim

 * @param m the mesh description
 **/
struct cell_info_t ****mesh_cellinfo_alloc(const struct mesh_t *m)
{
    struct cell_info_t ****c;

    int z, x, y;

    int nb_lat_cell,            /* number of cells in lattitude/longitude */
     nb_lon_cell;

    c = (struct cell_info_t ****)
        malloc(m->nlayers * sizeof(struct cell_info_t ***));
    assert(c);

#ifdef DEBUG
    fprintf(stderr, "cell_info_alloc(): allocating %d layers\n",
            m->nlayers);
#endif
    for (z = 0; z < m->nlayers; z++) {
        nb_lat_cell = m->layer[z]->nlat;
        nb_lon_cell = m->layer[z]->nlon;

#ifdef DEBUG
        fprintf(stderr,
                "cell_info_alloc(): layer %d : allocating %d cells in lat\n",
                z, nb_lat_cell);
        fprintf(stderr,
                "cell_info_alloc(): layer %d : allocating %d cells in lon\n",
                z, nb_lon_cell);
#endif

        c[z] = (struct cell_info_t ***)
            malloc(nb_lat_cell * sizeof(struct cell_info_t **));
        assert(c[z]);

        for (x = 0; x < nb_lat_cell; x++) {
            c[z][x] = (struct cell_info_t **)
                malloc(nb_lon_cell * sizeof(struct cell_info_t *));
            assert(c[z][x]);

            for (y = 0; y < nb_lon_cell; y++) {
                c[z][x][y] = NULL;
            }
        }
    }
    return (c);
}

/** \brief duplicate geographic coordinates 
 *
 *  FIXME : to be removed when point3d is replaced by coord_geo
 *  see comments at bottom of mesh_cellinfo_length_feed()
 **/
static
struct coord_geo_t *my_local_dup_geo_coord(const struct coord_geo_t *p)
{
    struct coord_geo_t *np;

    np = (struct coord_geo_t *) malloc(sizeof(struct coord_geo_t));
    assert(np);
    np->lat = p->lat;
    np->lon = p->lon;
    np->prof = p->prof;
    return (np);
}

/*
 *\brief insert length information into a cell. 
 *
 * Fill in a given cell with length information.
 *
 * PRECOND : the cell is allocated cell_info_t zone.
 *
 * @param c  the cell 
 * @param layer the layer
 * @param x x cell coord 
 * @param y y cell coord 
 * @param in first point in the cell
 * @param out last point in the cell
 * @param P_length P length  
 * @param S_length S length 
 * @param rid the ray id 
**/
void mesh_cellinfo_length_feed(struct cell_info_t *c,
                               const struct coord_geo_t *in,
                               const struct coord_geo_t *out,
                               const double P_length,
                               const double S_length, const long int rid)
{

    int n = 0;                  /* previous number of items */

    n = c->nitems;

    c->item = (struct cell_info_item_t *)
        realloc(c->item, (n + 1) * sizeof(struct cell_info_item_t));
    assert(c->item);

    c->item[n].rayid = rid;
    c->item[n].P_length = P_length;
    c->item[n].S_length = S_length;

    /* alloc since in and out will be freed */
    c->item[n].in = my_local_dup_geo_coord(in);
    c->item[n].out = my_local_dup_geo_coord(out);
    /* FIXME : original dup_geo_coord() was taken from libray when this 
     * function was in ray2mesh. Now, we should not establish a new dependency 
     * with libray (i.e. keep libmesh and libray independant). Hence, we should
     * use the dup_geo_cord() of libmesh, which does not exists because of 
     * point_3d existence. Until point3d is replaced by geo_coord, use a copy
     * of the dup_geo_coord() code (it is placed just before this function).
     * steph - 21 Feb 2004 */
    c->nitems++;
}

/**\brief Given two cell ids c0 and c1, return the c0 face    
 * number adjacent to the c1 cell.                    
 *
 * Note that the 2 cells may be differing by at most one of 
 * their cell_id's subscript (e.g. c0_{2,4,0} and c1_{2,5,0}) 
 * otherwise they would be neighbour by a edge (e.g. c0_{0,0,0} 
 * and c1_{1,1,0}) or by a corner (e.g. c0_{0,0,0} and c1_{1,0,0}) 
 * or not neighbour (e.g. c0_{0,0,0} and c1_{1,1,1}).
 * There is an exeption when the origin meridian is between the two cells, 
 * (e.g c0_{0,0,0} and c1_{0,39,0}, and the mesh have a lat width of 40 cells)
 *
 * The face returned may be one of 
 * NORTH_FACE, SOUTH_FACE, EAST_FACE, WEST_FACE, UP_FACE, DOWN_FACE.
 *
 * When an error occured face may be one of :
 * 	-1 : c0 and c1 cells are not adjacent
 * 	-2 : c0 and c1 are the same cell 
 * 
 * PRECOND : (|c0.x-c1.x| == 1) xor (|c0.y-c1.y| == 1) xor (|c0.z-c1.z| == 1) 
 *           xor (origin meridian case)
 *
 * @param mesh mesh description
 * @param c0 first cell's coord
 * @param c1 second cell's coord     
 **/
int
mesh_cellinfo_facehit_get(const struct mesh_t *mesh,
                          const struct coord_z3_t *c0,
                          const struct coord_z3_t *c1)
{
    /* use cell_ids (int) coordinates */
    int c0x = c0->x;            /* working copy */

    int c0y = c0->y;            /* working copy */

    int c1x = c1->x;            /* working copy */

    int c1y = c1->y;            /* working copy */

    int dx = abs(c0->x - c1->x);

    int dy = abs(c0->y - c1->y);

    int dz = abs(c0->z - c1->z);

    /* Let the extent (number of cells) in latitude be n ,
     * Currently n is stored in mesh->layer[l]->nlat  where l is c0->z or c1->z.
     * 
     * A distance(c0->x,c1->x) = n
     * implies c0->x and c1->x are neighours (toroidally). In other word :
     * (i)   0 <= c0->x , c1->x < n 
     * (ii)  abs(c0->x - c1->x) = n - 1
     * (i) and (ii) => (c0->x = 0 and c1->x = n-1) or (c1->x = 0 and (c0->x = n-1)
     *
     * same thing applies for longitude.
     */

    if (dx == mesh->layer[c0->z]->nlat - 1) {
        /* modifies working copy value to reflect neighbourhood */
        if (c1x > c0x)
            c1x = -1;
        else
            c0x = -1;
        dx = 1;
    }

    if (dy == mesh->layer[c0->z]->nlon - 1) {
        if (c1y > c0y)
            c1y = -1;
        else
            c0y = -1;
        dy = 1;
    }

    if (!dx && !dy && !dz) {
        /*fprintf(stderr,
           "Internal error, c0(x=%d,y=%d,z=%d) and c1(x=%d,y=%d,z=%d) are the same cell\n",
           c0->x, c0->y, c0->z, c1->x, c1->y, c1->z); */
        /* same cell */
        return (-2);
    }

    if (dx > 1 || dy > 1 || dz > 1 || dx + dy + dz > 1) {
        /* PRECOND broken => return -1 
           (probably too long segment that missed a cell) */
        /*fprintf(stderr,
           "%s, too long segment : c0(x=%d,y=%d,z=%d) and c1(x=%d,y=%d,z=%d) !\n",
           __FUNCTION__, c0->x, c0->y, c0->z, c1->x, c1->y, c1->z);
         */
        return (-1);
    }

    /* depth */
    if (c0->z > c1->z) {
        return (UP_FACE);
    }
    if (c0->z < c1->z) {
        return (DOWN_FACE);
    }

    /* latitude ie. x */
    if (c0x > c1x) {
        /* c0 south face */
        return (SOUTH_FACE);
    }
    if (c0x < c1x) {
        /* c0 north face */
        return (NORTH_FACE);
    }

    /* longitude ie. y */
    if (c0y < c1y) {
        return (EAST_FACE);
    } else {
        return (WEST_FACE);
    }

}

/**
 * \brief Update hit counts on cell faces.
 *
 * Given the mesh desc. mesh, the cell_info c    
 * and two cells c0 and c1 of c,  fill in the c0 and c1 fields with  
 * hit face info.                                                   
 *
 * PRECOND : c0 and c1 are allocated and all c0 and c1 fields x,y, and z 
 *           are positive.
 *
 * @param mesh the light mesh
 * @param c the cell_info
 * @param c0 first hit cell
 * @param c1 adjacent cell to c0 also hit
**/
void
mesh_cellinfo_facehit_feed(const struct mesh_t *mesh,
                           struct cell_info_t ****c,
                           const struct coord_z3_t *c0,
                           const struct coord_z3_t *c1)
{
    int face;

#ifdef DEBUG
    fprintf(stderr,
            "mesh_cellinfo_facehit_feed(): feeding adjacent cells ");
    fprintf(stderr, "c[z=%d,x=%d,y=%d] ", c0->z, c0->x, c0->y);
    fprintf(stderr, "(and partially c1 c[z=%d,x=%d,y=%d])\n", c1->z, c1->x,
            c1->y);
#endif

    face = mesh_cellinfo_facehit_get(mesh, c0, c1);
    assert(face >= 0);
    c[c0->z][c0->x][c0->y]->faces_hit[face]++;

    /* symetric case : only if cell is not a border cell */
    /*if (c[c1->z][c1->x][c1->y]) { */
    face = mesh_cellinfo_facehit_get(mesh, c1, c0);
    assert(face >= 0);
    c[c1->z][c1->x][c1->y]->faces_hit[face]++;
    /*} */
#ifdef DEBUG
    fprintf(stderr, "hit faces for c[z=%d,x=%d,y=%d]\n", c0->z, c0->x,
            c0->y);
    dump_facehit_info(stderr, c[c0->z][c0->x][c0->y]);
#endif

}

/**\brief Like feed_cell_hit_info() but when the cell has no neighbour.
 *
 * Given the cell_info c, the cell 
 * coordinates c0, the last point gp0 in c0 cell, and first point gp1
 * out of the cell, determines which face of c0 has been hit by the 
 * segment [gp0,gp1] and modify the hit_face counter of c0 in c.
 *
 * PRECOND : c0 is allocated.
**/
void mesh_cellinfo_singlefacehit_feed(const struct mesh_t *mesh,
                                      struct cell_info_t ****c,
                                      const struct coord_z3_t *c0,
                                      const struct coord_z3_t *c1)
{
    int face;

    face = mesh_cellinfo_facehit_get(mesh, c0, c1);
    assert(face >= 0);
    c[c0->z][c0->x][c0->y]->faces_hit[face]++;
}

/* 
 * \brief Updates a block in a given cell. 
 *
 * Given a pointer to a cell_info element, and a block id
 * look if this block has already been traversed (hit). If the block  
 * id is not found, then realloc the block list with one more element 
 * and stores the block id in the new element. If the block id is     
 * found, just return. The ci->nblocks contains in the end the number 
 * of different cell blocks hit by rays.                              
 * @param ci a cell of the mesh
 * @param block_id the block id
**/
void mesh_cellinfo_block_feed(struct cell_info_t *ci, blocknum_t block_id)
{
    int i;

    if (ci->block_hit) {
        for (i = 0; i < ci->nblocks; i++)
            if (block_id == ci->block_hit[i])
                return;
    }

    ci->nblocks++;
    ci->block_hit = (blocknum_t *) realloc
        (ci->block_hit, ci->nblocks * sizeof(blocknum_t));
    assert(ci->block_hit);
    ci->block_hit[ci->nblocks - 1] = block_id;
}

/** \brief operates a translation in rays ids stored in a cell_info.
 * 
 * returns the number of cells processed
 * 
 * @param c the light mesh
 * @param m the mesh description
 * @param incr the value to add to each rayid 
 */
int mesh_cellinfo_rayids_increment(struct cell_info_t ****c,
                                   const struct mesh_t *m, const int incr)
{
    struct cell_info_t *ci;

    int i, x, y, z;

    int nb = 0;

    if (!c)
        return (0);
    for (z = 0; z < m->nlayers; z++) {
        for (x = 0; x < m->layer[z]->nlat; x++) {
            for (y = 0; y < m->layer[z]->nlon; y++) {
                ci = c[z][x][y];        /* shorthand */
                if (ci && ci->nitems) { /* allocated cells with nitems=0 are orphan cells : ignored */
                    nb++;
                    for (i = 0; i < ci->nitems; i++) {
                        ci->item[i].rayid += incr;
                    }
                }
            }
        }
    }

    return (nb);
}

/* -----------------------------------------------------------
 *
 *                       DUMP METHODS
 *
 * -----------------------------------------------------------
 */

/**\brief prints out the face hit fields                 
 *
 * @param fd the file descriptor
 * @param cell the cell whose face hit is to be printed 
 **/
void mesh_cellinfo_facehit_dump(FILE * fd, struct cell_info_t *cell)
{
    int f;

    if (cell->faces_hit) {
        fprintf(fd, "faces hits [");
        for (f = 0; f < NB_HIT_FACES; f++)
            fprintf(fd, "%d ", cell->faces_hit[f]);
        fprintf(fd, "] (%d rays)\n", cell->nitems);
    }
}

/**\brief prints out the block hit fields               
 *
 * @param fd the file descriptor
 * @param cell the cell whose block hit is to be printed 
 **/
static void mesh_cellinfo_blockhit_dump(FILE * fd,
                                        struct cell_info_t *cell)
{
    int b;

    if (cell->block_hit) {
        fprintf(fd, "block hits [");
        for (b = 0; b < cell->nblocks; b++)
            fprintf(fd, "%d ", cell->block_hit[b]);
        fprintf(fd, "] (%d blocks) score=%.2f\n", cell->nblocks,
                cell->score[DISP]);
    }
}

/**\brief prints out cell's scores information and some statistics in an human readable format 
 *
   @param fd the file descriptor 
   @param c the cell_info to dump 
   @param m the mesh description 
 **/
void mesh_cellinfo_dump(FILE * fd,
                        struct cell_info_t ****c, struct mesh_t *m)
{
    struct cell_info_t *ci;

    int i, x, y, z;

    int cnt = 0, cnt_full = 0;

    int nb_lat_cell;            /* number of cells in lattitude/longitude */

    int nb_lon_cell;

    float length;

    if (!c) {
        fprintf(stderr, "dump_cell_info: no cell to dump !\n");
        return;
    }
    fprintf(fd,
            "dump_cell_info()   faces hits is [down up north south east west]\n");
    fprintf(fd,
            "format : cell_id score hit_score length_score disp_score nbrays\n");

    for (z = 0; z < m->nlayers; z++) {
        nb_lat_cell = m->layer[z]->nlat;
        nb_lon_cell = m->layer[z]->nlon;

        fprintf(fd, "[layer %d]\n", z);

        for (x = 0; x < nb_lat_cell; x++) {
            for (y = 0; y < nb_lon_cell; y++) {
                /* lenght per ray code info */
                ci = c[z][x][y];        /* shorthand */
                if (ci && ci->nitems) { /* allocated cells with nitems=0 are orphan cells : ignored */
                    cnt_full++;

                    fprintf(fd, "[%d,%d,%d] %.2f %.2f %.2f %.2f %d\n",
                            x, y, z,
                            ci->score[SCORE], ci->score[HIT],
                            ci->score[LENGTH], ci->score[DISP],
                            ci->nitems);

                    /* face hit information */
                    mesh_cellinfo_facehit_dump(fd, ci);
                    mesh_cellinfo_blockhit_dump(fd, ci);

                    for (i = 0; i < ci->nitems; i++) {
                        length =
                            ci->item[i].P_length + ci->item[i].S_length;
                        fprintf(fd,
                                "\t-->%.2f kms (P=%.2f,S=%.2f), in=(%.2f %.2f %.2f) out=(%.2f %.2f %.2f) (rid=%ld)\n",
                                length, ci->item[i].P_length,
                                ci->item[i].S_length, ci->item[i].in->lat,
                                ci->item[i].in->lon, ci->item[i].in->prof,
                                ci->item[i].out->lat, ci->item[i].out->lon,
                                ci->item[i].out->prof, ci->item[i].rayid);
                    }

                    mesh_cellinfo_free(c[z][x][y]);
                    c[z][x][y] = NULL;

                    fprintf(fd, "\n");
                }
                cnt++;
            }
        }
    }
    fprintf(fd, "[%d/%d cells traversed (%.2f %%)]\n", cnt_full, cnt,
            ((float) cnt_full / (float) cnt) * 100);
}

/**\brief prints out cell's scores information in SCO format 
 *
 * prints out the cell's scores information in SCO format which consist in
 * one line per cell, sorted by layer, with the latitude, longitude, layer, and
 * all the scores (Composit score, hit, length, geometric dispersion, nb of rays)
 *
 * <B>nbscores {scores_names_i}</B> (header) <BR>
 * <B>[lat,lon,layer] {scores_i}</B>
 * <B>...</B>
 *
 * Example :
   \verbatim
   5 Score Hit Length Disp nbrays
   [0,9,0] 0.01 0.17 0.07 0.05 7
   [0,10,0] 0.01 0.17 0.07 0.05 2
   [0,11,0] 0.02 0.17 0.09 0.06 13
   [0,12,0] 0.01 0.17 0.04 0.05 6
   ...
   \endverbatim

   @param filename name file
   @param c the cell_info to dump 
   @param m the mesh description 
**/
int mesh_cellinfo_write_sco(char *filename,
                            struct cell_info_t ****c, struct mesh_t *m)
{
    int i;

    int x, y, z;

    int nb_lat_cell, nb_lon_cell;       /* number of cells in latitude/longitude */

    int nb_cells = 0;

    FILE *fd;

    if (!c) {
        fprintf(stderr, "mesh_cellinfo_write_sco(): no cell to dump !\n");
        return (0);
    }

    if (!(fd = fopen(filename, "w"))) {
        perror(filename);
        exit(1);
    }

    fprintf(fd, "# format=sco, generated by %s v%s, mesh=%s\n",
            PACKAGE, VERSION, m->xml_filename);
    fprintf(fd, "%d ", NB_R2M_SCORE);
    for (i = 0; i < NB_R2M_SCORE; i++) {
        fprintf(fd, "%s ", R2M_SCORE_NAME[i]);
    }
    fprintf(fd, "\n");

    for (z = 0; z < m->nlayers; z++) {
        nb_lat_cell = m->layer[z]->nlat;
        nb_lon_cell = m->layer[z]->nlon;

        for (x = 0; x < nb_lat_cell; x++) {
            for (y = 0; y < nb_lon_cell; y++) {
                if (c[z][x][y] && c[z][x][y]->nitems) {
                    /* allocated cells with nitems=0 are orphan -> ignored */
                    nb_cells++;
                    /*fprintf(fd, "[%d,%d,%d] %.2f %.2f %.2f %.2f %d\n", */
                    fprintf(fd, "[%d,%d,%d] %g %g %g %g %d\n",
                            x, y, z,
                            c[z][x][y]->score[SCORE],
                            c[z][x][y]->score[HIT],
                            c[z][x][y]->score[LENGTH],
                            c[z][x][y]->score[DISP], c[z][x][y]->nitems);
                }
            }
        }
    }

    fclose(fd);
    return (nb_cells);
}

/**\brief Is a point in a cell's mesh. If yes, which cell id ?
 *
 * Given geographic coordinates (lat,lon,depth) in degrees and depth
 * return an int code depending on the fact that the point is/is_not in 
 * the mesh and if the point is in the mesh. The cell coordinates 
 * the point belong to is set in cell_id.
 *
 * Take care of longitude value which should be compatible with 
 * the mesh lon_min, lon_max.
 * 
 * POSTCOND : - return 1 if point (lat,lon,depth) is in mesh and cell_id is 
 *            modified  
 *            - or return [0,-1,..-5] if point (lat,lon,depth) is not in the 
 *            mesh. The returned value indicates *only* where the test fails
 *            and *not* which face is traversed !
 **/
int mesh_cellinfo_point2cell(const struct mesh_t *mesh,
                             const double lat, const double lon,
                             const double depth,
                             struct coord_z3_t *cell_id)
{
    int nb_lat_cell, nb_lon_cell;

    float cell_width_lat, cell_width_lon;

    int ret_code = mesh_cellinfo_which_layer(mesh, depth, &(cell_id->z));

    if (ret_code < 0) {
        /* the point is below/on top of the mesh : 
         * (ret_code,cell_id->z) == (-DOWN_FACE,-1) | (-UP_FACE,mesh->nlayer)
         * indicates which direction it went out of the mesh */
        return (ret_code);
    }

    /* some bounds values */
    nb_lat_cell = mesh->layer[cell_id->z]->nlat;        /* total nb of cells in lat */
    nb_lon_cell = mesh->layer[cell_id->z]->nlon;        /* total nb of cells in lon */

    /* cell width in degree in lat and lon */
    cell_width_lat =
        mesh->layer[cell_id->z]->lat_unit * mesh->parameter->lat_unit_size;
    cell_width_lon =
        mesh->layer[cell_id->z]->lon_unit * mesh->parameter->lon_unit_size;

    cell_id->x =
        (int) floor((lat - mesh->parameter->lat_min) / cell_width_lat);
    if (cell_id->x >= nb_lat_cell)
        return (-NORTH_FACE);
    if (cell_id->x < 0)
        return (-SOUTH_FACE);

    cell_id->y =
        (int) floor((lon - mesh->parameter->lon_min) / cell_width_lon);
    if (cell_id->y >= nb_lon_cell)
        return (-EAST_FACE);
    if (cell_id->y < 0)
        return (-WEST_FACE);

    return (1);
}

/**
 * \brief returns mesh layer to which a point belongs to
 *
 * given the mesh parameters contained in mesh, and a   
 * depth z, returns 0 if the depth is in the mesh and the \param layer 
 * is set to the layer number (positive or null int) the point belongs to.
 *
 * The returned value is a negative int if the depth z is not in mesh. 
 * If z is less deeper than 1st layer, returns -UP_FACE, and sets \param layer 
 * to -1.
 * If z is deeper than deeper layer, returns -DOWN_FACE and sets \param layer 
 * to last layer number + 1 
 **/
int mesh_cellinfo_which_layer(const struct mesh_t *mesh,

                                                /**< mesh description */

                              const double z,   /**< depth to check */
                              int *layer)
{

/**< layer returned for z */
    int i;

    if (z < mesh->layer[0]->zstart) {
        *layer = -1;
        return (-UP_FACE);
    }
    if (z > mesh->layer[mesh->nlayers - 1]->zend) {
        *layer = mesh->nlayers;
        return (-DOWN_FACE);
    }
    /* from here, z is in mesh */
    for (i = 0; i < mesh->nlayers; i++) {
        if ((z >= mesh->layer[i]->zstart) && (z < mesh->layer[i]->zend)) {
            *layer = i;
            return (i);
        }
    }
    /* assert(i>=0); */
    return (-99);               /* should not reach this point */
}

/** \brief find the max ray id in the light mesh 
 **/
long int mesh_cellinfo_get_max_rayid(struct cell_info_t ****c,
                                     struct mesh_t *m)
{
    int i;

    int x, y, z;

    int nb_lat_cell, nb_lon_cell;       /* number of cells in latitude/longitude */

    long int max_ray_id = 0;

    for (z = 0; z < m->nlayers; z++) {
        nb_lat_cell = m->layer[z]->nlat;
        nb_lon_cell = m->layer[z]->nlon;

        for (x = 0; x < nb_lat_cell; x++) {
            for (y = 0; y < nb_lon_cell; y++) {
                if (c[z][x][y] && c[z][x][y]->nitems) {
                    /* allocated cells with nitems=0 are orphan -> ignored */
                    for (i = 0; i < c[z][x][y]->nitems; i++) {
                        if (c[z][x][y]->item[i].rayid > max_ray_id) {
                            max_ray_id = c[z][x][y]->item[i].rayid;
                        }
                    }
                }
            }
        }
    }

    return (max_ray_id);
}
