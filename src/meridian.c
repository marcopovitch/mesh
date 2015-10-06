#include <stdio.h>
#include "const.h"
#include "point3d.h"
#include "cell.h"
#include "mesh.h"
#include "meridian.h"
#include "layer.h"
#include "extern.h"

/*-----------------------------------------------------------------*/

/* make_meridian : generates a meridian of cells given a layer    */

/*-----------------------------------------------------------------*/
struct cell_t *make_meridian(struct layer_t *layer, struct mesh_t *mesh)
{
    struct cell_t *c_pred = NULL;

    struct cell_t *c = NULL;

    struct cell_t *first_cell = NULL;

    struct coord_z3_t cell_id;

    int n;

    struct mesh_parameter_t *mp;

    mp = mesh->parameter;

    if (DEBUG)
        fprintf(stderr,
                "make_meridian in layer(id=%d): nb cells to make  = %d\n",
                layer->number, layer->nlat);

    /* must be updated later when running in mesh */
    cell_id.x = 0;
    cell_id.y = 0;
    cell_id.z = 0;

    n = 0;
    while (n < layer->nlat) {
        c = new_cell(mesh->allocated_cells, &cell_id);
        layer->ncells++;
        if (c_pred) {
            c_pred->neighbour_list[NSD_1] =
                add_neighbour_to_cell(c_pred, c, NSD_1);
            c->neighbour_list[NSD_2] =
                add_neighbour_to_cell(c, c_pred, NSD_2);
        } else {
            first_cell = c;
        }
        c_pred = c;
        n++;
    }
    return (first_cell);
}

/*-----------------------------------------------------------------*/

/* link_meridian_1 : generates the links between 2 meridians       */

/*                 where  0<= (lon1 & lon2) <180                   */

/*-----------------------------------------------------------------*/
int link_meridian_1(struct cell_t *c1, struct cell_t *c2)
{
    int nb = 0;

    do {
        /* link east/west */
        c1->neighbour_list[EAST_D] = add_neighbour_to_cell(c1, c2, EAST_D);
        c2->neighbour_list[WEST_D] = add_neighbour_to_cell(c2, c1, WEST_D);
        /* next cells */
        c1 = get_cell_from_list(c1->neighbour_list[NSD_1], 0);
        c2 = get_cell_from_list(c2->neighbour_list[NSD_1], 0);
        nb++;
    }
    while (c1 && c2);
    return (nb);
}

/*----------------------------------------------------------------*/

/* link_meridian_2 : generates the links between 2 meridians      */

/*                 where  0<=lon1<180  & 180 <= lon2 < 360        */

/*----------------------------------------------------------------*/
int link_meridian_2(struct cell_t *c1, struct cell_t *c2)
{
    int nb = 0;

    while (c2->neighbour_list[NSD_1])
        c2 = get_cell_from_list(c2->neighbour_list[NSD_1], 0);

    do {
        /* link east/west */
        c1->neighbour_list[EAST_D] = add_neighbour_to_cell(c1, c2, EAST_D);
        c2->neighbour_list[WEST_D] = add_neighbour_to_cell(c2, c1, WEST_D);
        /* next cells */
        c1 = get_cell_from_list(c1->neighbour_list[NSD_1], 0);
        c2 = get_cell_from_list(c2->neighbour_list[NSD_2], 0);
        nb++;
    }
    while (c1 && c2);
    return (nb);
}

/*----------------------------------------------------------------*/

/* link_meridian_3 : generates the links between 2 meridians      */

/*                 where  180<= lon1 < 360  & 0 <= lon2 < 180     */

/*----------------------------------------------------------------*/
int link_meridian_3(struct cell_t *c1, struct cell_t *c2)
{
    int nb = 0;

    while (c1->neighbour_list[NSD_1])
        c1 = get_cell_from_list(c1->neighbour_list[NSD_1], 0);

    do {
        /* link east/west */
        c1->neighbour_list[EAST_D] = add_neighbour_to_cell(c1, c2, EAST_D);
        c2->neighbour_list[WEST_D] = add_neighbour_to_cell(c2, c1, WEST_D);
        /* next cells */
        c1 = get_cell_from_list(c1->neighbour_list[NSD_2], 0);
        c2 = get_cell_from_list(c2->neighbour_list[NSD_1], 0);
        nb++;
    }
    while (c1 && c2);
    return (nb);
}
