#ifndef __LAYER_H__
#define __LAYER_H__

/** \brief layer definition */
struct layer_t {

    char *name;                 /**< layer name for userfriendliness */

    int number;                 /**< number of this layer            */

    long int ncells;                    /**< number of cells in the layer    */

    int nlat;                   /**< number of cell in latitude      */

    int nlon;                   /**< number of cell in longitude     */

    long int npoints;           /**< number of allocated points      */

    struct cell_t *cell;        /**< initial cell of the layer       */

    int lat_unit;               /**< nb of plat (ie cell min)        */

    int lon_unit;               /**< nb of plon (ie cell min)        */

    float zstart;               /**< where the layer starts          */

    float zend;                 /**< where the layer ends            */
};

int link_layer(struct mesh_t *mesh, struct layer_t *layer_up,
               struct layer_t *layer_down);
void make_layer(struct layer_t *layer, struct mesh_t *mesh);

void dump_layer(struct layer_t *layer, struct mesh_t *mesh);

void layer_info(char *txt, struct layer_t *layer);

int get_north_direction(struct cell_t *c, struct layer_t *layer,
                        struct mesh_t *mesh);
int cells_are_in_the_same_crown(struct cell_t *c1, struct cell_t *c2);
#endif
