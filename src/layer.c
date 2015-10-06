#include <stdio.h>
#include "const.h"
#include "cell.h"
#include "mesh.h"
#include "meridian.h"
#include "layer.h"
#include "modulo.h"
#include "extern.h"

/**
 * \brief Asses if a real value is in the intervall [inf,sup]
 */
int is_in_interval(float val, float bound_inf, float bound_sup)
{
    if (val < bound_inf)
        return (0);
    if (val > bound_sup)
        return (0);
    return (1);
}

/** \brief link layers together */

/** return the number of links created */
int link_layer(struct mesh_t *mesh, struct layer_t *layer_up,
               struct layer_t *layer_down)
{
    struct mesh_parameter_t *mp;

    struct layer_t *l1, *l2;

    int nlon, nlat;

    int sens;

    int nb_links = 0;

    int north;

    struct cell_t *p1, *p2;

    struct cell_t *c1, *c2;

    mp = mesh->parameter;

    if (layer_up->zstart > layer_down->zstart) {
        l1 = layer_up;
        l2 = layer_down;
        sens = DOWN_D;          /* l1->neighbour = l2 which is DOWN_D */
    } else {
        l1 = layer_down;
        l2 = layer_up;
        sens = UP_D;
    }

    if (l1->nlat != l2->nlat || l1->nlon != l2->nlon) {
        fprintf(stdout,
                "link_layer: layers parameters differ, link disable\n");
        return (0);
    }

    /* move on all cell from l1 */
    c1 = p1 = l1->cell;
    c2 = p2 = l2->cell;
    /* define the north */
    north = get_north_direction(NULL, l1, mesh);

    for (nlat = 0; nlat < l1->nlat && c1; nlat++) {
        for (nlon = 0; nlon < l1->nlon && c1; nlon++) {
            /* do the job */
            c1->neighbour_list[sens] = add_neighbour_to_cell(c1, c2, sens);
            c2->neighbour_list[opposite_dir(sens)] =
                add_neighbour_to_cell(c2, c1, opposite_dir(sens));
            nb_links++;

            /* go to next cell */
            c1 = get_cell_from_list(c1->neighbour_list[EAST_D], 0);
            c2 = get_cell_from_list(c2->neighbour_list[EAST_D], 0);
        }

        c1 = p1 = get_cell_from_list(p1->neighbour_list[north], 0);
        c2 = p2 = get_cell_from_list(p2->neighbour_list[north], 0);
    }

    return (nb_links);
}

/** \brief link layers together */

/** FIXME */
int link_layer_bis(struct mesh_t *mesh, struct layer_t *layer_up,
                   struct layer_t *layer_down)
{
    struct mesh_parameter_t *mp;

    struct layer_t *l1, *l2;

    struct cell_t *c1, *c2, *c2_init;   /* cells */

    struct cell_t *m1, *m2;     /* refere to first cell of a meridian */

    struct cell_t *m1_init, *m2_init;

    int nb_meridians, nb1_meridians, nb2_meridians;

    int nb_parallels, nb1_parallels, nb2_parallels;

    int nlon, nlat;

    int sens;

    mp = mesh->parameter;

    /* longitude */
    if (layer_up->lon_unit >= layer_down->lon_unit) {
        l1 = layer_up;
        l2 = layer_down;
        sens = DOWN_D;          /* l1->neighbour = l2 which is DOWN_D */
    } else {
        l1 = layer_down;
        l2 = layer_up;
        sens = UP_D;
    }

    m1 = m1_init = l1->cell;
    m2 = m2_init = l2->cell;
    nb1_meridians = (int) ((mp->lon_max - mp->lon_min)
                           / (l1->lon_unit * mp->lon_unit_size));
    nb2_meridians = l1->lon_unit / l2->lon_unit;

    /* latitude */
    if (layer_up->lat_unit >= layer_down->lat_unit) {
        l1 = layer_up;
        l2 = layer_down;
    } else {
        l1 = layer_down;
        l2 = layer_up;
    }

    nb1_parallels = (int) ((mp->lat_max - mp->lat_min)
                           / (l1->lat_unit * mp->lat_unit_size));
    nb2_parallels = l1->lat_unit / l2->lat_unit;

    if (VERBOSE) {
        fprintf(stderr, "link_layer %p %p :\n", layer_up, layer_down);
        fprintf(stderr, "nb meridians = %d,   nb2_meridians = %d\n",
                nb1_meridians, nb2_meridians);
        fprintf(stderr, "nb parallels = %d, nb2_parallels = %d\n",
                nb1_parallels, nb2_parallels);
    }

    nb_meridians = 0;
    do {
        fprintf(stderr, "nb_meridians =%d/%d\n", nb_meridians,
                nb1_meridians - 1);
        c1 = m1;
        c2 = m2;
        nb_parallels = 0;

        do {
            fprintf(stderr, "  nb_parallels =%d/%d\n", nb_parallels,
                    nb1_parallels - 1);

            /*----------------------*/
            /* here begins the link */

            /*----------------------*/
            nlat = 0;
            do {
                fprintf(stderr, "    nlat  =%d/%d\n", nlat,
                        nb2_parallels - 1);
                nlon = 0;
                c2_init = c2;
                while (nlon != nb2_meridians && c2) {
                    fprintf(stderr, "      nb nlon =%d/%d\n", nlon,
                            nb2_meridians - 1);
                    c1->neighbour_list[sens] =
                        add_neighbour_to_cell(c1, c2, sens);
                    c2->neighbour_list[opposite_dir(sens)] =
                        add_neighbour_to_cell(c2, c1, opposite_dir(sens));
                    /* nect c2 */
                    c2 = get_cell_from_list(c2->neighbour_list[EAST_D], 0);
                    nlon++;
                    fprintf(stderr, "      fin nb nlon\n");
                }

                c2 = c2_init;
                nlat++;

                if (nlat != nb2_parallels)
                    c1 = get_cell_from_list(c1->neighbour_list[NSD_1], 0);
            }
            while (nlat != nb2_parallels);

            /* go to the next parallel for link  */
            if (!c1 || !c2)
                break;
            c1 = get_cell_from_list(c1->neighbour_list[NSD_1], 0);
            c2 = get_cell_from_list(c2->neighbour_list[NSD_1], 0);
            nb_parallels++;

        }
        while (nb_parallels != nb1_parallels);

        /* go to the next meridian */
        m1 = get_cell_from_list(m1->neighbour_list[EAST_D], 0);
        m2 = get_cell_from_list(m2->neighbour_list[EAST_D], 0);
        nb_meridians++;

    }
    while (nb_meridians != nb1_meridians);

    if (VERBOSE) {
        fprintf(stderr, "m1 neighbour list (UP_D)\n");
        dump_cell_list(m1_init->neighbour_list[UP_D]);
        fprintf(stderr, "m1 neighbour list (DOWN_D)\n");
        dump_cell_list(m1_init->neighbour_list[DOWN_D]);
    }

    return (0);
}

/*-----------------------------------------------------------------*/

/* make_layer :                                                    */

/*-----------------------------------------------------------------*/
void make_layer(struct layer_t *layer, struct mesh_t *mesh)
{

    struct cell_t *last_m = NULL;

    struct cell_t *first_m = NULL;

    struct cell_t *m = NULL;

    struct cell_t *c1, *c2, *first_cell;

    struct cell_t *cell2return = NULL;

    struct mesh_parameter_t *mp;

    int n, nb_meridian, nb_max_meridian, nl;

    float c1_lon;

    float plon;                 /* lon step */

    mp = mesh->parameter;

    layer->ncells = 0;
    nb_meridian = layer->nlon;
    nb_max_meridian = (int) (360. / (layer->lon_unit * mp->lon_unit_size));

    if (DEBUG)
        fprintf(stderr, "make_layer : nb meridians to make = %d\n",
                nb_meridian);

    n = 0;
    c1_lon = mp->lon_min;
    plon = mp->lon_unit_size * layer->lon_unit;

    while (n < nb_meridian) {
        m = make_meridian(layer, mesh);

        if (first_m == NULL) {
            first_m = m;
            if (is_in_interval(modulo360(mp->lon_min, 0), 0., 180. - plon)) {
                if (DEBUG)
                    fprintf(stderr, "make_layer : init cell in [0-180[\n");
                layer->cell = first_m;
            } else {
                if (DEBUG)
                    fprintf(stderr,
                            "make_layer : init cell in [180-360[\n");
                cell2return = first_m;
                while (cell2return->neighbour_list[NSD_1]) {
                    cell2return =
                        get_cell_from_list(cell2return->neighbour_list
                                           [NSD_1], 0);
                }
                layer->cell = cell2return;
            }

        } else {
            if (modulo360(c1_lon, 0) > 180. - EPS
                && modulo360(c1_lon, 0) < 180. + EPS) {
                if (DEBUG)
                    fprintf(stderr,
                            "make_layer : link_meridian_2 (%.1f-%.1f)\n",
                            c1_lon, c1_lon + plon);
                nl = link_meridian_2(last_m, m);
            } else if (modulo360(c1_lon, 1) > 360. - EPS
                       && modulo360(c1_lon, 1) < 360. + EPS) {
                if (DEBUG)
                    fprintf(stderr,
                            "make_layer : link_meridian_3 (%.1f-%.1f)\n",
                            c1_lon, c1_lon + plon);
                nl = link_meridian_3(last_m, m);
            } else {
                if (DEBUG)
                    fprintf(stderr,
                            "make_layer : link_meridian_1 (%.1f-%.1f)\n",
                            c1_lon, c1_lon + plon);
                nl = link_meridian_1(last_m, m);
            }
            if (DEBUG)
                fprintf(stderr, "make_layer : nb links made = %d\n", nl);
        }
        last_m = m;
        n++;
        c1_lon += plon;
    }

    /*----------------------------*/
    /* link last & first meridian */

    /*----------------------------*/
    if ((int) (mp->lon_max - mp->lon_min) == 360) {
        if (DEBUG)
            fprintf(stderr, "make_layer : link first and last meridian\n");
        if (is_in_interval
            (0., modulo360(c1_lon, 0) - EPS,
             modulo360(c1_lon + plon, 0) - EPS)) {
            if (DEBUG)
                fprintf(stderr,
                        "make_layer : link_meridian_3 (%.1f-%.1f)\n",
                        c1_lon, c1_lon + plon);
            link_meridian_3(last_m, first_m);
        } else
            if (is_in_interval
                (180., modulo360(c1_lon, 0) - EPS,
                 modulo360(c1_lon + plon, 1) - EPS)) {
            if (DEBUG)
                fprintf(stderr,
                        "make_layer : link_meridian_2 (%.1f-%.1f)\n",
                        c1_lon, c1_lon + plon);
            link_meridian_2(last_m, first_m);
        } else {
            if (DEBUG)
                fprintf(stderr,
                        "make_layer : link_meridian_1 (%.1f-%.1f)\n",
                        c1_lon, c1_lon + plon);
            link_meridian_1(last_m, first_m);
        }
    }

    /*return; */

    /*---------------------*/
    /* link South Pole ... */

    /*---------------------*/
    if ((int) (mp->lat_min) == -90) {
        if (DEBUG)
            fprintf(stderr, "make_layer : link south (topo)\n");

        if (cell2return) {
            c1 = first_cell = cell2return;
        } else {
            c1 = first_cell = first_m;
        }
        c1_lon = mp->lon_min;
        plon = mp->lon_unit_size * layer->lon_unit;

        /* dump couronne */
        /*if (DEBUG) {
           n = 0;
           fprintf(stderr, "\ncouronne = ");
           c2 = c1;
           while (n < nb_meridian) {
           n++;
           fprintf(stderr, "%p(%d), ", c2, n);
           c2 = get_cell_from_list(c2->neighbour_list[EAST_D], 0);
           }
           fprintf(stderr, "\n");
           } */

        nl = 0;                 /* number of move to complete the pole link */
        do {
            if (c1_lon + 180. < mp->lon_max - plon + EPS) {
                /* link is possible */

                /* we go on the cell to link with c1 */
                n = 0;
                c2 = c1;
                if (VERBOSE)
                    fprintf(stderr,
                            "make_layer : going to cell c2 to link with c1 : ");
                while (n < nb_max_meridian / 2) {       /*  nb_max_meridian must be = 2*k */
                    if (VERBOSE)
                        fprintf(stderr, "+");
                    c2 = get_cell_from_list(c2->neighbour_list[EAST_D], 0);
                    n++;
                }
                if (VERBOSE)
                    fprintf(stderr, "\n");

                /*if (DEBUG)
                   fprintf(stderr, "with c2 = %p\n", c2);
                 */

                /* the link */
                if (is_in_interval
                    (modulo360(c1_lon, 0), 0. - EPS, 180. - EPS)
                    && !is_in_interval(180., modulo360(c1_lon, 0) - EPS,
                                       modulo360(c1_lon + plon,
                                                 1) - EPS)) {
                    if (VERBOSE) {
                        fprintf(stderr, "make_layer : c1 is in [0,180[\n");
                        fprintf(stderr, "make_layer : c1=%p c2=%p\n", c1,
                                c2);
                        fprintf(stderr,
                                "make_layer : c1->nsd2 = %p, c2->nsd1 = %p\n",
                                get_cell_from_list(c1->neighbour_list
                                                   [NSD_2], 0),
                                get_cell_from_list(c2->neighbour_list
                                                   [NSD_1], 0));
                    }

                    c1->neighbour_list[NSD_2] =
                        add_neighbour_to_cell(c1, c2, NSD_2);
                    c2->neighbour_list[NSD_1] =
                        add_neighbour_to_cell(c2, c1, NSD_1);
                    /*c2->neighbour_list[NSD_2] =
                       add_neighbour_to_cell(c2, c1, NSD_2); */

                    if (VERBOSE) {
                        fprintf(stderr, "make_layer : c1=%p c2=%p\n", c1,
                                c2);
                        fprintf(stderr,
                                "make_layer : c1->nsd2 = %p, c2->nsd1 = %p\n",
                                get_cell_from_list(c1->neighbour_list
                                                   [NSD_2], 0),
                                get_cell_from_list(c2->neighbour_list
                                                   [NSD_1], 0));
                    }

                } else {
                    if (VERBOSE) {
                        fprintf(stderr,
                                "make_layer : c1 is in [180,360\n");
                    }
                    c1->neighbour_list[NSD_1] =
                        add_neighbour_to_cell(c1, c2, NSD_1);
                    c2->neighbour_list[NSD_2] =
                        add_neighbour_to_cell(c2, c1, NSD_2);
                    /*c2->neighbour_list[NSD_1] =
                       add_neighbour_to_cell(c2, c1, NSD_1); */
                }
            }

            c1 = get_cell_from_list(c1->neighbour_list[EAST_D], 0);
            c1_lon += plon;
            nl++;
        }
        while (c1 && nl < nb_meridian / 2);
    }

    /*----------------*/
    /* and North Pole */

    /*----------------*/
    if ((int) (mp->lat_max) == 90) {
        if (DEBUG)
            fprintf(stderr, "make_layer : link north (topo)\n");

        c1 = first_m;
        if (!cell2return) {
            while (c1->neighbour_list[NSD_1]) {
                c1 = get_cell_from_list(c1->neighbour_list[NSD_1], 0);
            }
        }

        /* dump couronne */
        /*if (DEBUG) {
           n = 0;
           fprintf(stderr, "\ncouronne = ");
           c2 = c1;
           while (n < nb_meridian) {
           n++;
           fprintf(stderr, "%p(%d), ", c2, n);
           c2 = get_cell_from_list(c2->neighbour_list[EAST_D], 0);
           }
           fprintf(stderr, "\n");
           } */

        first_cell = c1;
        c1_lon = mp->lon_min;
        plon = mp->lon_unit_size * layer->lon_unit;

        nl = 0;                 /* number of move to complete the pole link */
        do {
            if (c1_lon + 180. < mp->lon_max - plon + EPS) {
                /* link is possible */
                /*if (DEBUG)
                   fprintf(stderr,
                   "link possible for cell c1 = %p (lon=%.1f)",
                   c1, c1_lon);
                 */

                /* we go on the cell to link with c1 */
                n = 0;
                c2 = c1;
                if (VERBOSE)
                    fprintf(stderr,
                            "make_layer : going to cell c2 to link with c1 : ");
                while (n < nb_max_meridian / 2) {
                    if (VERBOSE)
                        fprintf(stderr, "+");
                    c2 = get_cell_from_list(c2->neighbour_list[EAST_D], 0);
                    n++;
                }
                if (VERBOSE)
                    fprintf(stderr, "\n");

                /*if (DEBUG)
                   fprintf(stderr, "with c2 = %p\n", c2);
                 */

                /* the link */
                if (is_in_interval
                    (modulo360(c1_lon, 0), 0. - EPS, 180. - EPS)
                    && !is_in_interval(180., modulo360(c1_lon, 0) - EPS,
                                       modulo360(c1_lon + plon,
                                                 1) - EPS)) {
                    if (VERBOSE)
                        fprintf(stderr, "make_layer : c1 is in [0,180[\n");
                    c1->neighbour_list[NSD_1] =
                        add_neighbour_to_cell(c1, c2, NSD_1);
                    c2->neighbour_list[NSD_2] =
                        add_neighbour_to_cell(c2, c1, NSD_2);
                    /*c2->neighbour_list[NSD_1] =
                       add_neighbour_to_cell(c2, c1, NSD_1); */
                } else {
                    if (VERBOSE)
                        fprintf(stderr,
                                "make_layer : c1 is in [180,360\n");
                    c1->neighbour_list[NSD_2] =
                        add_neighbour_to_cell(c1, c2, NSD_2);
                    c2->neighbour_list[NSD_1] =
                        add_neighbour_to_cell(c2, c1, NSD_1);
                    /*c2->neighbour_list[NSD_2] =
                       add_neighbour_to_cell(c2, c1, NSD_2); */
                }
            }
            c1 = get_cell_from_list(c1->neighbour_list[EAST_D], 0);
            c1_lon += plon;
            nl++;
        }
        while (c1 && nl < nb_meridian / 2);
    }
}

/*-----------------------------------------------------------------*/

/* dump_layer :                                                    */

/*-----------------------------------------------------------------*/
void dump_layer(struct layer_t *layer, struct mesh_t *mesh)
{

    int nbcrown, nblon;

    struct cell_t *clat, *clon, *last_clat;

    int direction;

    struct mesh_parameter_t *mp;

    mp = mesh->parameter;

    direction = get_north_direction(NULL, layer, mesh);

    /* initial cell always in the lower(south)/ left(west) */
    clat = layer->cell;
    nbcrown = 0;
    do {
        clon = clat;
        nblon = 0;
        fprintf(stderr, "-> ");
        do {
            fprintf(stderr, "%p[z=%d,x=%d,y=%d] ", clon, clon->id.z,
                    clon->id.x, clon->id.y);
            clon = get_cell_from_list(clon->neighbour_list[EAST_D], 0);
            nblon++;

            if (clon == clat) {
                fprintf(stderr, ", stop CYCLING ");
            } else if (!clon) {
                fprintf(stderr, ", stop NULL ");
            }
        }
        while (clon && clon != clat);

        fprintf(stderr, "(%d cells)\n", nblon);
        last_clat = clat;
        clat = get_cell_from_list(clat->neighbour_list[direction], 0);
        nbcrown++;
        if (!clat)
            break;

        if (cells_are_in_the_same_crown(clat, last_clat)) {
            fprintf(stderr, "stop CYCLING on crown\n");
        } else if (!clat) {
            fprintf(stderr, "stop NULL on crown\n");
        }
    }
    while (!cells_are_in_the_same_crown(clat, last_clat) && clat);
    fprintf(stderr, "** dump_layer : nb of crown %d **\n", nbcrown);
}

/*-----------------------------------------------------------------*/

/* layer_info :                                                    */

/*-----------------------------------------------------------------*/
void layer_info(char *txt, struct layer_t *layer)
{
    fprintf(stderr,
            "%s\tname='%s', number=%d\n\tnlat = %d, nlon = %d\n\tzstart=%.2f, zend=%.2f\n\tncells=%ld firstcell=%p, nballocpoint=%ld\n\tlat_unit=%d, lon_unit=%d\n\taddr=%p\n",
            txt, layer->name, layer->number, layer->nlat, layer->nlon,
            layer->zstart, layer->zend, layer->ncells, layer->cell,
            layer->npoints, layer->lat_unit, layer->lon_unit, layer);
}

int get_north_direction_orig(struct cell_t *c, struct layer_t *layer,
                             struct mesh_t *mesh)
{
    float plon;

    int ptnbr;

    struct mesh_parameter_t *mp;

    mp = mesh->parameter;
    plon = mp->lon_unit_size * layer->lon_unit;

    if (c) {
        ptnbr = c->point[0];
        plon = plon + mesh->allocated_points->tab[ptnbr]->lon;
    }

    if (mp->lon_min < 180. - EPS &&
        !is_in_interval(180.,
                        modulo360(mp->lon_min, 0) - EPS,
                        modulo360(mp->lon_min + plon, 1) - EPS)) {
        return (NSD_1);
    } else {
        return (NSD_2);
    }
}

/**
 * \brief get_north_direction : indicate the north pole 
 *
 * get_north_direction : indicate the north pole         
 * return NSD_1, or NSD_2 in respect to                   
 * the cell'longitude                                      
 * if cell is provided compute it's north direction         
 * if *not*, compute the north direction of first cell in    
 * the layer                                                 
 */
int get_north_direction(struct cell_t *c, struct layer_t *layer,
                        struct mesh_t *mesh)
{
    float lon, plon;

    int ptnbr;

    struct mesh_parameter_t *mp;

    int pt_id;

    mp = mesh->parameter;
    plon = mp->lon_unit_size * layer->lon_unit;

    /* to be sure of the result on the edge take the mean on longitude */
    if (c) {
        switch (c->id.x) {
        case 0:
            pt_id = 3;          /* the point 0 is at pole, take care of the link */
            break;
        default:
            pt_id = 0;
        }

        ptnbr = c->point[pt_id];
        lon = mesh->allocated_points->tab[ptnbr]->lon;

        /*fprintf(stderr, "[x=%d,y=%d,z=%d] lon = %f ", 
           c->id.x, c->id.y, c->id.z, lon); */
    } else {
        lon = mp->lon_min;
        /* fprintf(stderr, "[x=?,y=?,z=?] lon = %f ", 
           lon); */
    }

    /*if ( is_in_interval (modulo360(lon, 0), 0., 180. - plon)) */
    if (is_in_interval(modulo360(lon + plon / 2., 0), 0., 180.)) {
        /* fprintf(stderr, "%s\n", DIRECTION[NSD_1]); */
        return (NSD_1);
    } else {
        /* fprintf(stderr, "%s\n", DIRECTION[NSD_2]); */
        return (NSD_2);
    }
}

/*----------------------------------------------*/

/* is_in_the_same_crown                         */

/* return true if 2 cells are in the same crown */

/*----------------------------------------------*/
int cells_are_in_the_same_crown(struct cell_t *c1, struct cell_t *c2)
{
    struct cell_t *c;

    /* try to the EAST_D */
    c = c1;
    do {
        c = get_cell_from_list(c->neighbour_list[EAST_D], 0);
        if (!c)
            break;
        if (c == c2)
            return (1);
    } while (c && c != c1);

    if (c == c1)
        return (0);

    /* try to the WEST_D */
    c = c1;
    do {
        c = get_cell_from_list(c->neighbour_list[WEST_D], 0);
        if (!c)
            break;
        if (c == c2)
            return (1);
    } while (c && c != c1);

    return (0);
}
