#include "cell.h"
#include "mesh.h"
#include "layer.h"
#include "merge.h"

/* merge-matrix creation 
 * given a layer of nlat x nlob cells this matrix size is
 * 2.nlat x nlon elements
 */

int **merge_matrix_alloc(int l, int c)
{
    int **M;

    int i;

    M = (int **) malloc(2 * l * sizeof(int *));
    for (i = 0; i < 2 * c; i++) {
        M[i] = (int *) calloc(sizeof(int), c);
    }
    return (M);
}

/* 1st step : very basic stuff
 * we feed the merge-matrix with 1 or 0 depending 
 * on cell's score value neigbourhood 
 * 1 if the cell's score are nearly the same , 0 if not 
 *
 * score_rank is the score to work on (cell->score[score_rank])
 */
void feed_merge_matrix(int **F, int score_rank, struct layer_t *layer,
                       struct mesh_parameter_t *mp)
{
    struct cell_t *c_init, *c, *c_east, *c_north;

    int i, j, dir;

    int k, l;

    int **M;

    M = merge_matrix_alloc(layer->nlat, layer->nlon);

    /* run over all layer's cells */
    k = l = 0;
    c = layer->cell;
    for (i = 0; i < layer->nlat; i++) {
        c = c_init;

        for (j = 0; j < layer->nlon; j++) {
            /* look how cell's neighbourood is similar */
            c_east = get_cell_from_list(c->neighbour_list[EAST], 0);

            /* warning :  find a way to do that */
            dir = get_north_direction(c, layer, mp);
            c_north = get_cell_from_list(c->neighbour_list[dir], 0);

            if (c_east)
                if (c->score[score_rank] == (c_east->score[score_rank]))
                    M[l][k] = 1;

            if (c_north)
                if (c->score[score_rank] == (c_north->score[score_rank]))
                    M[l + 1][k] = 1;

            c = c_east;
            k++;
        }
        c_init = get_cell_from_list(c_init->neighbour_list[dir], 0);
        l += 2;
    }

}
