#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <stdio.h>
#include <string.h>
#include "const.h"
#include "cell.h"
#include "mesh.h"
#include "layer.h"
#include "modulo.h"
#include "extern.h"

/**\brief returns version of library                   
 **/
char *libmeshversion(void)
{
    char *s;

    s = malloc(strlen(PACKAGE) + strlen(VERSION) + 3);
    sprintf(s, "%s-%s", PACKAGE, VERSION);
    return (s);
}

/** \brief remove all data structure associated with the mesh, but not
 * the cells.
 **/
void free_mesh(struct mesh_t *mesh)
{
    int l, f;

    int i;

    /* layers */
    for (l = 0; l < mesh->nlayers; l++) {
        if (mesh->layer[l]) {
            if (mesh->layer[l]->name)
                free(mesh->layer[l]->name);
            free(mesh->layer[l]);
        }
    }
    free(mesh->layer);

    /* data section */
    for (f = 0; f < NB_MESH_FILE_FORMAT; f++) {
        if (!mesh->data[f]) {
            continue;
        }

        for (l = 0; l < mesh->data[f]->ndatafile; l++) {
            free(mesh->data[f]->filename[l]);
        }
        free(mesh->data[f]->filename);
        free(mesh->data[f]->format);
        free(mesh->data[f]->directory);
        free(mesh->data[f]);
    }
    free(mesh->data);
    mesh->data = NULL;

    /* points */
    if (mesh->allocated_points->tab) {
        for (i = 0; i < mesh->allocated_points->nbpoint; i++) {
            destroy_point3d(mesh->allocated_points->tab[i]);
            mesh->allocated_points->tab[i] = NULL;
        }
        free(mesh->allocated_points->tab);
        mesh->allocated_points->tab = NULL;
        mesh->allocated_points->nbpoint = 0;
    }
    free(mesh->allocated_points);
    mesh->allocated_points = NULL;
    /*fprintf(stderr,"free'd %d points\n", i); */

    /* cell */
    if (mesh->allocated_cells->tab) {
        for (i = 0; i < mesh->allocated_cells->nbcell; i++) {
            destroy_cell(mesh->allocated_cells->tab[i]);
            mesh->allocated_cells->tab[i] = NULL;
        }
        free(mesh->allocated_cells->tab);
        mesh->allocated_cells->tab = NULL;
        mesh->allocated_cells->nbcell = 0;

    }
    free(mesh->allocated_cells);
    mesh->allocated_cells = NULL;
    /*fprintf(stderr,"free'd %d cells\n", i); */

    /* xml file name */
    free(mesh->xml_filename);

    /* mesh parameters */
    free(mesh->parameter);
    mesh->parameter = NULL;

    /* free mesh */
    free(mesh);
    mesh = NULL;
}

/**\brief Set up mesh connectivity and metrics.
 *
 *
 * Given the mesh with its initial parameters, modify the mesh contents
 * by setting  up the internal mesh layers including cells connectivity 
 * and metrics. 
 * 
 * @param mesh the bare mesh structure containing the initial parameters 
 */
void make_mesh(struct mesh_t *mesh)
{
    int l;                      /* the layer */

    int first_layer = -1;       /* id of the first_layer */

    struct cell_t *c, *c_lat_ini;

    float lat, lon, prf, pprf;

    struct mesh_parameter_t *mp = mesh->parameter;

    float plat, plon;

    int nb_meridians, nb_cell_in_meridian;

    int nmerid, nlatit;

    int direction;

    fprintf(stdout, "mesh construction (%s)\n", mesh->xml_filename);
    mesh->ncells = 0;

    /************************/
    /* making mesh topology */

    /************************/
    for (l = 0; l < mesh->nlayers; l++) {

        if (!mesh->layer[l]) {
            if (VERBOSE)
                fprintf(stderr, "** Nothing to do for layer %d **\n", l);
            continue;
        }

        make_layer(mesh->layer[l], mesh);
        mesh->ncells += mesh->layer[l]->ncells;

        if (first_layer == -1) {
            first_layer = l;
            mesh->cell = mesh->layer[l]->cell;  /* old compatibility */
        }

        if (VERBOSE) {
            layer_info("*** Created layer\n", mesh->layer[l]);
        }

        if (l != first_layer) {
            int nb;

            nb = link_layer(mesh, mesh->layer[l - 1], mesh->layer[l]);
            if (VERBOSE) {
                fprintf(stderr, "link_layer: nb cell linked = %d\n", nb);
            }
        }
    }

    /**********************/
    /* making mesh metric */

    /**********************/
    if (VERBOSE)
        fprintf(stderr, "\n*** metric ***\n");

    /* we process each layers */
    for (l = 0; l < mesh->nlayers; l++) {
        if (!mesh->layer[l]) {
            continue;
        }

        /* ... and set initial value */
        lat = mp->lat_min;
        prf = mesh->layer[l]->zstart;

        pprf = mesh->layer[l]->zend - mesh->layer[l]->zstart;
        plat = mp->lat_unit_size * mesh->layer[l]->lat_unit;
        plon = mp->lon_unit_size * mesh->layer[l]->lon_unit;

        nb_meridians = mesh->layer[l]->nlon;
        nb_cell_in_meridian = mesh->layer[l]->nlat;

        direction = get_north_direction(NULL, mesh->layer[l], mesh);

        c_lat_ini = c = mesh->layer[l]->cell;
        nlatit = 0;             /* latitude cell counter */

        while (c && nlatit < nb_cell_in_meridian) {     /* next crown */

            lon = modulo360(mp->lon_min, 0);
            nmerid = 0;
            while (nmerid < nb_meridians) {

                if (ALLOC_ALL_POINTS) {
                    create_points_for_cell(c, lat, lon, prf, plat, plon,
                                           pprf, NULL,
                                           mesh->allocated_points, NULL);
                } else {
                    updateinfo_cell(c, lat, lon, prf,
                                    plat, plon, pprf,
                                    mesh->allocated_points);
                }

                /* cell id */
                c->id.x = nlatit;
                c->id.y = nmerid;
                c->id.z = l;

                /* just a test */
                /*{
                   long int lci;
                   int x,y,z;
                   lci = linearize_cell_id (&(c->id), mesh);
                   unlinearize_cell_id (lci, &x,&y,&z, mesh);

                   if (c->id.x != x || c->id.y != y || c->id.z != z) {
                   fprintf(stderr, "x=%d/%d y=%d/%d z=%d/%d\n", 
                   c->id.x, x, 
                   c->id.y, y,
                   c->id.z, z);
                   exit(1);
                   }

                   } */

                if (DEBUG)
                    fprintf(stderr, "cell id [z=%d,x=%d,y=%d]\n", c->id.z,
                            c->id.x, c->id.y);

                c = get_cell_from_list(c->neighbour_list[EAST_D], 0);
                lon += plon;
                nmerid++;
            }

            mesh->layer[l]->npoints = mesh->allocated_points->nbpoint;
            c = c_lat_ini =
                get_cell_from_list(c_lat_ini->neighbour_list[direction],
                                   0);
            nlatit++;
            lat += plat;
        }
    }
}

/** \brief prints out mesh information for debugging goal
 */
void mesh_dump(struct mesh_t *mesh)
{
    int l;                      /*  layers id */

    if (mesh && !mesh->cell) {
        fprintf(stderr, "mesh initialized but not constructed\n");
        return;
    }

    for (l = 0; l < mesh->nlayers; l++) {
        if (!mesh->layer[l])
            continue;
        fprintf(stderr, "\n** Layer %d\n", l);
        dump_layer(mesh->layer[l], mesh);
    }
}

 /** \brief dump_mesh_parameter : outputs the values of mesh parameters
 *
 * The ouput has the same format as the xml tags attributes. You can
 * output an xml format using the pretext and posttext variables 
 * (print "<mesh " and ">" in the pretxt and posttxt respectively)
 *
 * @param fd file descriptor of output
 * @param pretxt text to append before parameters
 * @param posttxt text to append after parameters
 * @param mp parameters themselves 
 *
 */
void mesh_dump_parameter(FILE * fd, char *pretxt, char *posttxt,
                         struct mesh_parameter_t *mp)
{
    fprintf(fd,
            "%s\tlat-min=\"%.2f\" lat-max=\"%.2f\"\n\tlon-min=\"%.2f\" lon-max=\"%.2f\"\n\tlat-unit-size=\"%.2f\" lon-unit-size=\"%.2f\"%s\n",
            pretxt,
            mp->lat_min, mp->lat_max,
            mp->lon_min, mp->lon_max, mp->lat_unit_size,
            mp->lon_unit_size, posttxt);
}

void mesh_dump_data(struct mesh_data_t *md)
{
    int i;

    fprintf(stderr, "format = %s, nb file = %d\n", md->format,
            md->ndatafile);
    fprintf(stderr, "directory = %s\n", md->directory);
    for (i = 0; i < md->ndatafile; i++) {
        fprintf(stderr, "\t%s\n", md->filename[i]);
    }
}

/** \brief move_in_mesh from start_cell to [lat,lon,layer] cell
 *
 * return a pointer to the cell with [lat,lon,layer] coordinates, 
 * return NULL if the coord is out-of the mesh 
 */

struct cell_t *move_in_mesh(struct mesh_t *mesh,
                            struct cell_t *start_cell, int lat,
                            int lon, int layer)
{
    int direction;

    struct cell_t *c;

    int lat_step, lon_step, layer_step;

    /* plan futur deplacement relatif */

    c = start_cell;

    if (!c) {
        /*fprintf(stderr, "move_in_mesh: warning start_cell is null !\n"); */
        c = mesh->layer[layer]->cell;
    }

    lat_step = lat - c->id.x;
    lon_step = lon - c->id.y;
    layer_step = layer - c->id.z;

    if (DEBUG) {
        fprintf(stderr, "c->x = %d c->y = %d c->z = %d\n", c->id.x,
                c->id.y, c->id.z);
        fprintf(stderr, "move lat=%d lon=%d layer=%d \n", lat_step,
                lon_step, layer_step);
        dump_cell(mesh->allocated_points, c, "cell_start");
    }

    /* layer */
    /* warning :  the link between layer is NOT efective */
    if (layer_step > 0) {
        direction = DOWN_D;
    } else {
        direction = UP_D;
        layer_step *= -1;
    }
    if (DEBUG)
        fprintf(stderr, "going to %s in %d steps\n",
                DIRECTION[direction], layer_step);

    if (layer_step != 0) {
        fprintf(stderr,
                "move_in_mesh : warning bad start_cell (not in the same layer)\n");
        exit(1);
    }

    /*if (DEBUG)
       dump_cell(mesh->allocated_points, c, "CELL a z"); */

    /************/
    /* latitude */

    /************/
    direction = get_north_direction(NULL, mesh->layer[c->id.z], mesh);
    if (lat_step < 0) {
        direction = opposite_dir(direction);
        lat_step *= -1;
    }
    if (DEBUG) {
        fprintf(stderr, "going to %s in %d steps\n", DIRECTION[direction],
                lat_step);
    }

    while (lat_step != 0) {
        /*fprintf(stderr, "[%d,%d,%d]\n", c->id.x, c->id.y, c->id.z); */
        c = get_cell_from_list(c->neighbour_list[direction], 0);
        if (!c) {
            /*fprintf(stderr, "move_in_mesh: error c = %p\n", c); */
            return (NULL);
            /*exit(1); */
        }
        lat_step--;
    }
    /*if (DEBUG)
       dump_cell(mesh->allocated_points, c, "CELL lat"); */

    /*************/
    /* longitude */

    /*************/
    if (lon_step > 0) {
        direction = EAST_D;
    } else {
        direction = WEST_D;
        lon_step *= -1;
    }

    if (DEBUG)
        fprintf(stderr, "going to %s in %d steps\n",
                DIRECTION[direction], lon_step);

    while (lon_step != 0) {
        /*fprintf(stderr, "[%d,%d,%d] , ", c->id.x, c->id.y, c->id.z); */
        c = get_cell_from_list(c->neighbour_list[direction], 0);
        if (!c) {
            /*fprintf(stderr, "move_in_mesh: error c = %p\n", c); */
            return (NULL);
            /*exit(1); */
        }
        lon_step--;
    }

    /*if (DEBUG)
       dump_cell(mesh->allocated_points, c, "CELL end"); */

    return (c);
}

/** \brief linearise the cell_id to a unique integer
 *
 * the cell id is set following :
 * 	each layer,
 * 	each latitude,
 * 	each longitude.
 **/
long int linearize_cell_id(struct coord_z3_t *cell_id,
                           const struct mesh_t *mesh)
{
    int l;

    long int count;

    long int lci;

    int nbx = 0, nby = 0;

    assert(mesh);
    assert(cell_id);

    count = 0;
    for (l = 0; l < cell_id->z; l++) {
        if (!mesh->layer[l])
            continue;
        nbx = mesh->layer[l]->nlat;     /* total nb of cells in lat */
        nby = mesh->layer[l]->nlon;     /* total nb of cells in lon */
        count += nbx * nby;
    }

    /* in the current layer */
    /*nbx = mesh->layer[cell_id->z]->nlat; *//* total nb of cells in lat */
    nby = mesh->layer[cell_id->z]->nlon;

    lci = count + (cell_id->x * nby) + cell_id->y;
    return (lci);
}

/** \brief return cell id given it's linearized cell id
 */
void unlinearize_cell_id(long int lcid, int *x, int *y, int *z,
                         const struct mesh_t *mesh)
{
    int nbx, nby, l;

    long int count, old_count;

    /* found layer */
    count = old_count = 0;
    for (l = 0; l < mesh->nlayers; l++) {
        nbx = mesh->layer[l]->nlat;
        nby = mesh->layer[l]->nlon;
        count += nbx * nby;

        if (count > lcid) {
            break;
        }
        old_count = count;
    }

    *z = l;

    nbx = mesh->layer[l]->nlat;
    nby = mesh->layer[l]->nlon;

    *z = l;
    *x = (lcid - old_count) / nby;
    *y = lcid - old_count - (*x) * nby;
}

/** \brief shows the differents sections (SPARSE,RES,IRM,R2M,SCO) 
 * and associated files in a XML file */
void mesh_show_sections(struct mesh_t *mesh)
{
    int i, s;

    assert(mesh);

    for (s = 0; s < NB_MESH_FILE_FORMAT; s++) {
        if (!mesh->data[s]) {
            continue;
        }

        fprintf(stdout, "section %s : %d file(s)\n", MESH_FILE_FORMAT[s],
                mesh->data[s]->ndatafile);
        for (i = 0; i < mesh->data[s]->ndatafile; i++) {
            fprintf(stdout, "\t%s\n", mesh->data[s]->filename[i]);
        }
    }
}

/** \brief add an associated file to a given section in a mesh 
 */
void mesh_add_data_filename(struct mesh_t *mesh, int format,
                            char *filename)
{
    int i;

    assert(mesh);

    /* store into the mesh the filename */
    if (!mesh->data[format]) {
        mesh->data[format] =
            (struct mesh_data_t *) calloc(1, sizeof(struct mesh_data_t));
        mesh->data[format]->format = strdup(MESH_FILE_FORMAT[format]);
        assert(mesh->data[format]->format);
        mesh->data[format]->directory = strdup(".");
        assert(mesh->data[format]->directory);
        mesh->data[format]->ndatafile = 0;
    } else {
        /* check the file is not already here */
        for (i = 0; i < mesh->data[format]->ndatafile; i++) {
            if (!strcmp(mesh->data[format]->filename[i], filename)) {
                fprintf(stderr, "%s already in the section\n", filename);
                return;
            }
        }
    }
    mesh->data[format]->ndatafile++;
    mesh->data[format]->filename =
        (char **) realloc(mesh->data[format]->filename,
                          sizeof(char *) * mesh->data[format]->ndatafile);
    assert(mesh->data[format]->filename);
    mesh->data[format]->filename[mesh->data[format]->ndatafile - 1] =
        strdup(filename);
}

/** \brief remove all associated file from a given section 
 */
void mesh_remove_data_entry(struct mesh_t *mesh, int format)
{
    int l;

    assert(!(format > NB_MESH_FILE_FORMAT || format < 0));

    if (mesh->data[format]) {
        for (l = 0; l < mesh->data[format]->ndatafile; l++) {
            free(mesh->data[format]->filename[l]);
        }
        free(mesh->data[format]->filename);
        free(mesh->data[format]->format);
        free(mesh->data[format]->directory);
        free(mesh->data[format]);
        mesh->data[format] = NULL;
    }
}

/** \brief return a string containing the name of a section 
 */
char *mesh_get_section_name(int format)
{
    assert(!(format > NB_MESH_FILE_FORMAT || format < 0));
    return (strdup(MESH_FILE_FORMAT[format]));
}
