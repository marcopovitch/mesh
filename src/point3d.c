#include "point3d.h"

/*----------------------------------------------------------------*/

/*   destroy_point3d :                                             */

/*----------------------------------------------------------------*/
void destroy_point3d(struct point3d_t *point3d)
{
    free(point3d);
}

/*----------------------------------------------------------------*/

/*   create_point3d :                                             */

/*   return the point ID                                           */

/*----------------------------------------------------------------*/
int
create_point3d(struct point3d_tab_t *tab, float lat, float lon, float prf)
{
    struct point3d_t *point;

    if (!tab)
        return (0);

    /* add a point */
    /* must use a better algo */
    point = (struct point3d_t *) malloc(sizeof(struct point3d_t));
    assert(point);

    point->lat = lat;
    point->lon = lon;
    point->prf = prf;

    tab = add_point3d_to_tab(tab, point);
    return (tab->nbpoint - 1);
}

/*----------------------------------------------------------------*/

/*   add_point3d_to_tab :                                         */

/*----------------------------------------------------------------*/
struct point3d_tab_t *add_point3d_to_tab(struct point3d_tab_t *tabpoint,
                                         struct point3d_t *point)
{

    if (tabpoint->nbpoint % tabpoint->blockminsize == 0) {
        /* allocate new block */
        tabpoint->tab = (struct point3d_t **) realloc(tabpoint->tab,
                                                      (tabpoint->nbpoint +
                                                       tabpoint->blockminsize)
                                                      *
                                                      sizeof(struct
                                                             point3d_t *));
        assert(tabpoint->tab);
    }
    tabpoint->tab[tabpoint->nbpoint] = point;
    tabpoint->nbpoint++;

    return (tabpoint);
}

/**
 * \brief outputs contents of point3d structure
 **/
void dump_point3d(char *s, struct point3d_t *p)
{
    fprintf(stderr, "%s: lat = %f lon = %f depth = %f\n",
            s, p->lat, p->lon, p->prf);
}
