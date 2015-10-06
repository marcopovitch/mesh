#include "mesh.h"
#include "layer.h"
#include "cell.h"
#include "metacell.h"
#include "extern.h"

/*
#define DEBUG_NGHB fprintf(stderr, "h:x=%d,y=%d,z=%d,m=%d,n=%d c:x=%d,y=%d,z=%d n:x=%d,y=%d,z=%d\n", \
					head_cell->id.x, head_cell->id.y, head_cell->id.z,	     \
					m,n,						  	     \
					c->id.x, c->id.y, c->id.z,			  	     \
					neighbour->id.x, neighbour->id.y, neighbour->id.z);	     \
		    fprintf(stderr, "hc_south=%s, north=%s, south=%s\n", 			     \
					  DIRECTION[hc_south], DIRECTION[north], DIRECTION[south]);  \
		    dump_cell_list(c->neighbour_list[NSD_1]);		\
		    dump_cell_list(c->neighbour_list[NSD_2]);		\
		    dump_cell_list(c->neighbour_list[EAST_D]);		\
		    dump_cell_list(c->neighbour_list[WEST_D]);		\
		    dump_cell_list(neighbour->neighbour_list[NSD_1]);	\
		    dump_cell_list(neighbour->neighbour_list[NSD_2]);	\
		    dump_cell_list(neighbour->neighbour_list[EAST_D]);	\
		    dump_cell_list(neighbour->neighbour_list[WEST_D]);
*/
#define DEBUG_NGHB

/** \brief surf into the irregular mesh structure, and find the neighbour
 *  cell list of each faces for all metacells.
 */
void metacell_find_neighbourhood(struct mesh_t *mesh)
{
    int i, j, l;

    int m, n;

    struct cell_t *head_cell, *c, *neighbour;

    int south, north, hc_north, hc_south;

    if (!mesh->nb_total_metacell) {
        fprintf(stdout,
                "%s: metacell_find_neighbourhood(): no metacell defined\n",
                __FILE__);
        return;
    }

    for (l = 0; l < mesh->nlayers; l++) {
        if (!mesh->metacell[l]) {
            fprintf(stdout,
                    "%s: metacell_find_neighbourhood(): no metacell in layer %d\n",
                    __FILE__, l);
            continue;
        }

        fprintf(stdout,
                "%s: metacell_find_neighbourhood(): layer %d working on %d metacells\n",
                __FILE__, l, mesh->nb_metacell[l]);
        for (i = 0; i < mesh->nb_metacell[l]; i++) {
            head_cell = mesh->metacell[l][i];
            m = head_cell->meta_scores[0].m;
            n = head_cell->meta_scores[0].n;

            if (head_cell->meta_neighbour_list) {
                fprintf(stderr,
                        "%s: metacell_find_neighbourhood(): neighbours already defined for cell[x=%d,y=%d,z=%d]\n",
                        __FILE__, head_cell->id.x, head_cell->id.y,
                        head_cell->id.z);
                exit(1);
            }

            /* allocate list to store neighbour */
            head_cell->meta_neighbour_list = (struct cell_list_t **)
                calloc(4, sizeof(struct cell_list_t *));

            c = head_cell;
            hc_north =
                get_north_direction(head_cell, mesh->layer[l], mesh);
            hc_south = opposite_dir(hc_north);

            /* go to the east:   find north neighbourhood */
            for (j = 0; j < m && c; j++) {
                north = get_north_direction(c, mesh->layer[l], mesh);
                south = opposite_dir(north);
                neighbour =
                    get_cell_from_list(c->neighbour_list[north], 0);
                if (neighbour && neighbour->meta_cell) {
                    head_cell->meta_neighbour_list[hc_north] =
                        add_cell_to_list(head_cell->meta_neighbour_list
                                         [hc_north], neighbour->meta_cell);
                    DEBUG_NGHB;
                    assert(head_cell != neighbour->meta_cell);
                }
                if (j != m - 1) {
                    /* only m-1 moves */
                    c = get_cell_from_list(c->neighbour_list[EAST_D], 0);
                }

            }
            assert(j == m);

            /* go to the south:  find east neighbourhood */
            north = get_north_direction(c, mesh->layer[l], mesh);
            south = opposite_dir(north);
            for (j = 0; j < n && c; j++) {
                neighbour =
                    get_cell_from_list(c->neighbour_list[EAST_D], 0);
                if (neighbour && neighbour->meta_cell) {
                    head_cell->meta_neighbour_list[EAST_D] =
                        add_cell_to_list(head_cell->meta_neighbour_list
                                         [EAST_D], neighbour->meta_cell);
                    DEBUG_NGHB;
                    assert(head_cell != neighbour->meta_cell);
                }
                if (j != n - 1) {
                    /* only n-1 moves */
                    c = get_cell_from_list(c->neighbour_list[south], 0);
                }
            }
            assert(j == n);

            /* go to the west:   find south neighbourhood */
            for (j = 0; j < m && c; j++) {
                north = get_north_direction(c, mesh->layer[l], mesh);
                south = opposite_dir(north);
                neighbour =
                    get_cell_from_list(c->neighbour_list[south], 0);
                if (neighbour && neighbour->meta_cell) {
                    head_cell->meta_neighbour_list[hc_south] =
                        add_cell_to_list(head_cell->meta_neighbour_list
                                         [hc_south], neighbour->meta_cell);
                    DEBUG_NGHB;
                    assert(head_cell != neighbour->meta_cell);
                }
                if (j != m - 1) {
                    /* only m-1 moves */
                    c = get_cell_from_list(c->neighbour_list[WEST_D], 0);
                }
            }
            assert(j == m);

            /* go to the north:  find west neighbourhood */
            for (j = 0; j < n && c; j++) {
                north = get_north_direction(c, mesh->layer[l], mesh);
                south = opposite_dir(north);
                neighbour =
                    get_cell_from_list(c->neighbour_list[WEST_D], 0);
                if (neighbour && neighbour->meta_cell) {
                    head_cell->meta_neighbour_list[WEST_D] =
                        add_cell_to_list(head_cell->meta_neighbour_list
                                         [WEST_D], neighbour->meta_cell);
                    DEBUG_NGHB;
                    assert(head_cell != neighbour->meta_cell);
                }
                if (j != n - 1) {
                    /* only n-1 moves */
                    c = get_cell_from_list(c->neighbour_list[hc_north], 0);
                }
            }
            assert(j == n);

            /* last check */
            if (c != head_cell) {
                fprintf(stderr, "%s: internal error (i=%d)!\n",
                        __FILE__, i);
                fprintf(stderr,
                        "h:x=%d,y=%d,z=%d,m=%d,n=%d c:x=%d,y=%d,z=%d\n",
                        head_cell->id.x, head_cell->id.y, head_cell->id.z,
                        m, n, c->id.x, c->id.y, c->id.z);
                exit(1);
            }

            /*{
               int nb_neighbours;
               long int lcid;

               lcid = linearize_cell_id ( &(head_cell->id), mesh);

               nb_neighbours = cell_list_get_nb_item (head_cell->meta_neighbour_list[NSD_1]);
               nb_neighbours+= cell_list_get_nb_item (head_cell->meta_neighbour_list[NSD_2]);
               nb_neighbours+= cell_list_get_nb_item (head_cell->meta_neighbour_list[WEST_D]);
               nb_neighbours+= cell_list_get_nb_item (head_cell->meta_neighbour_list[EAST_D]);
               fprintf(stderr, "%s: metacell_find_neighbourhood(): metacell lcid=%ld maille(%p) has %d neighbours\n", 
               __FILE__, lcid, head_cell, nb_neighbours);
               dump_cell_list(head_cell->meta_neighbour_list[NSD_1]);
               dump_cell_list(head_cell->meta_neighbour_list[NSD_2]);
               dump_cell_list(head_cell->meta_neighbour_list[WEST_D]);
               dump_cell_list(head_cell->meta_neighbour_list[EAST_D]);
               } */
        }

    }
}

/** \brief return in point_id array the height points id which
 *   delimit the metacell. point_id must be allocated.
 **/
void metacell_get_points_id(struct mesh_t *mesh,
                            struct cell_t *head_cell, int *point_id)
{
    int m, n;

    int l, j;

    int south, north;

    struct cell_t *c;

    assert(head_cell);

    m = head_cell->meta_scores[0].m;
    n = head_cell->meta_scores[0].n;
    l = head_cell->id.z;

    c = head_cell;

    point_id[3] = c->point[3];
    point_id[7] = c->point[7];

    /* go to the east */
    for (j = 0; j < m - 1 && c; j++) {
        c = get_cell_from_list(c->neighbour_list[EAST_D], 0);
    }
    assert(j == m - 1);

    point_id[2] = c->point[2];
    point_id[6] = c->point[6];

    /* go to the south */
    north = get_north_direction(c, mesh->layer[l], mesh);
    south = opposite_dir(north);
    for (j = 0; j < n - 1 && c; j++) {
        c = get_cell_from_list(c->neighbour_list[south], 0);
    }
    assert(j == n - 1);

    point_id[1] = c->point[1];
    point_id[5] = c->point[5];

    /* go to the west:   find south neighbourhood */
    for (j = 0; j < m - 1 && c; j++) {
        c = get_cell_from_list(c->neighbour_list[WEST_D], 0);
    }
    assert(j == m - 1);

    point_id[0] = c->point[0];
    point_id[4] = c->point[4];

    /* go to the north:  find west neighbourhood */
    north = get_north_direction(c, mesh->layer[l], mesh);
    for (j = 0; j < n - 1 && c; j++) {
        c = get_cell_from_list(c->neighbour_list[north], 0);
    }
    assert(j == n - 1);

    /* last check */
    if (c != head_cell) {
        fprintf(stderr,
                "%s: metacell_get_points_id(): internal error hcell=%p!\n",
                __FILE__, head_cell);
        fprintf(stderr, "h:x=%d,y=%d,z=%d c:x=%d,y=%d,z=%d\n",
                head_cell->id.x, head_cell->id.y, head_cell->id.z, c->id.x,
                c->id.y, c->id.z);
        exit(1);
    }

}
