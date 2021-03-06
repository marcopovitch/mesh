#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>

#ifdef __APPLE__
#include <malloc/malloc.h>
#else
#include <malloc.h>
#endif

#include "import.h"
#include "cellinfo.h"
#include "extern.h"
#include "convert_coord.h"
#include "modulo.h"

/** \brief compute offset to convert cell's coordinates from mesh m1 to mesh m2 for all layers
 **/
struct mesh_offset_t **compute_mesh_offset(struct mesh_t *m1,
                                           struct mesh_t *m2)
{
    struct mesh_offset_t **offset;

    int l;

    /* check mesh compatibility : FIXME */
    if (m1->nlayers != m2->nlayers) {
        fprintf(stderr,
                "compute_mesh_offset : nlayers must be the same (%d,%d), exiting !\n",
                m1->nlayers, m2->nlayers);
        exit(1);
    }
    offset = (struct mesh_offset_t **)
        calloc(m1->nlayers, sizeof(struct mesh_offset_t *));
    assert(offset);

    for (l = 0; l < m1->nlayers; l++) {
        if (m1->layer[l] && m2->layer[l]) {
            offset[l] = compute_mesh_offset_in_layer(m1, m2, l);
            /*
             * fprintf(stderr," offset layer %d: %d %d %d\n", l,
             * offset[l]->lat, offset[l]->lon,offset[l]->z);
             */
        }
    }

    return (offset);
}

/** \brief compute offset to convert cell's coordinates from mesh m1 to mesh m2 at a given
 *   layer
 **/
struct mesh_offset_t *compute_mesh_offset_in_layer(struct mesh_t *m1,
                                                   struct mesh_t *m2,
                                                   int l)
{
    struct mesh_offset_t *offset;

    int i;

    float lon1_min_a, lon2_min_a;

    float lon1_min_b, lon2_min_b;

    float delta[4];

    float delta_min;

    int max;

    assert(m1);
    assert(m2);
    assert(m1->layer);
    assert(m2->layer);

    assert(m1->layer[l]);
    assert(m2->layer[l]);

    lon1_min_a = m1->parameter->lon_min;
    lon2_min_a = m2->parameter->lon_min;

    /* usefull for modulo stuff */
    max = 360 / (m1->parameter->lon_unit_size * m1->layer[l]->lon_unit);

    /* longitudes must be in the same referential */
    if (lon2_min_a < -EPS) {
        lon2_min_b = lon2_min_a + 360;
    } else {
        lon2_min_b = lon2_min_a;
    }

    if (lon1_min_a < -EPS) {
        lon1_min_b = lon1_min_a + 360;
    } else {
        lon1_min_b = lon1_min_a;
    }

    delta[0] = lon2_min_a - lon1_min_a;
    delta[1] = lon2_min_b - lon1_min_b;
    delta[2] = lon2_min_a - lon1_min_b;
    delta[3] = lon2_min_b - lon1_min_a;

    delta_min = delta[0];
    for (i = 1; i < 4; i++) {
        /*
         * fprintf (stderr, "delta_min = %f delta[%d] = %f\n",
         * delta_min, i, delta[i]);
         */
        if (fabs(delta[i]) < fabs(delta_min)) {
            /* if (delta[i] >= 0 && delta[i] < delta_min) { */
            delta_min = delta[i];
        }
    }

    /* offset computation */
    offset = (struct mesh_offset_t *) malloc(sizeof(struct mesh_offset_t));
    assert(offset);

    offset->lat =
        (int) rint((m2->parameter->lat_min - m1->parameter->lat_min)
                   / (m1->layer[l]->lat_unit *
                      m1->parameter->lat_unit_size));

    offset->lon = (int) rint(modulo(delta_min /
                                    (m1->layer[l]->lat_unit *
                                     m1->parameter->lon_unit_size) + max,
                                    max));
    offset->z = 0;

    return (offset);
}

/** \brief Compare two cell's rayid to be used with qsort
 */
int compare_rayid(const void *a, const void *b)
{
    const struct cell_info_item_t *aa =
        (const struct cell_info_item_t *) a;
    const struct cell_info_item_t *bb =
        (const struct cell_info_item_t *) b;

    if ((*aa).rayid > (*bb).rayid)
        return 1;
    else if ((*aa).rayid < (*bb).rayid)
        return -1;
    else
        return 0;
}

/** \brief when importing r2m data file, ignore data carried by the next cell
 */
void fake_read_data(FILE * fd)
{
    int i;

    int tmp_nblocks;

    int tmp_nitems;

    for (i = 0; i < NB_HIT_FACES; i++) {
        fscanf(fd, "%*d");
    }
    fscanf(fd, "%d", &tmp_nblocks);
    for (i = 0; i < tmp_nblocks; i++) {
        fscanf(fd, "%*d");
    }
    fscanf(fd, "%d", &tmp_nitems);
    for (i = 0; i < tmp_nitems; i++) {
        fscanf(fd, "%*d %*f %*f %*f %*f %*f %*f %*f %*f");
    }
}

/** \brief Import r2m data file
 */
int
import2mesh_r2m_file(struct mesh_t *mesh, char *filename,
                     struct mesh_offset_t **offset)
{
    FILE *fd;

    struct cell_t *c;

    char line[256];

    int nb_cell;

    int x, y, z;

    int n, i;

    int bh;                     /* tmp blockhit */

    fprintf(stdout, "Importing r2m formated file '%s'\n", filename);
    if (!(fd = fopen(filename, "r"))) {
        perror(filename);
        exit(1);
    }
    /*
     * first line is commented but contains usefull information -
     * deprecated
     */
    fgets(line, 256, fd);

    /* number of non empty cells */
    fgets(line, 256, fd);
    sscanf(line, "%d", &nb_cell);

    for (n = 0; n < nb_cell; n++) {

        /* layer lat lon */
        fscanf(fd, "%d %d %d", &z, &x, &y);

        /* fprintf(stderr, "read %d %d %d\n", x,y,z); */

        /* translation */
        if (offset && offset[z]) {
            int mod;

            /* usefull for modulo stuff */
            mod =
                (360.0 /
                 (mesh->parameter->lon_unit_size *
                  mesh->layer[z]->lon_unit));

            /* z += offset->z; *//* z = layer_id */
            /*
             * fprintf(stderr, "z=%d offsety=%d, mod=%d\n", z,
             * offset[z]->lon, mod);
             */
            x += offset[z]->lat;
            y = modulo(y + offset[z]->lon + mod, mod);
        }
        /* fprintf(stderr, "aoc  %d %d %d\n", x,y,z); */

        /* check if user want this layer (command option) */
        if (!mesh->layer[z]) {
            fake_read_data(fd);
            continue;
        }
        /*
           fprintf(stderr, "%d %d %d max_x=%d max_y=%d\n", z, x, y,
           mesh->layer[z]->nlat, mesh->layer[z]->nlon );
         */

        /* attention au couche NULL */
        if (x < 0 || x >= mesh->layer[z]->nlat) {
            fake_read_data(fd);
            continue;
            /*
               fprintf(stderr,
               "%s: import2mesh_r2m_file(): x = %d, max x should be %d, exiting !\n",
               __FILE__, x, mesh->layer[z]->nlat);
               exit(1);
             */
        }
        if (y < 0 || y >= mesh->layer[z]->nlon) {
            fake_read_data(fd);
            continue;
            /*
             * fprintf(stderr, "import error : y = %d, max y
             * should be %d, exiting !\n", y,
             * mesh->layer[z]->nlon); exit(1);
             */
        }
        /* go the layer where lie the cell */
        c = mesh->layer[z]->cell;

        /* locate the [x=lat,y=lon,z=layer] cell */
        c = move_in_mesh(mesh, c, x, y, z);
        if (!c) {
            fprintf(stderr,
                    "move_in_mesh: error can't go to (x=%d,y=%d,z=%d)\n",
                    x, y, z);
            exit(1);
        }
        /* alloc cell info */
        if (!c->cell_info) {
            c->cell_info =
                (struct cell_info_t *) calloc(1,
                                              sizeof(struct cell_info_t));
            assert(c->cell_info);
            c->cell_info->item = NULL;
            c->cell_info->score = NULL;

        } else {
            fprintf(stderr,
                    "Cell info [z=%d,x=%d,y=%d] is already set, exiting !\n",
                    z, x, y);
            exit(1);
        }

        /* hit faces */
        c->cell_info->faces_hit =
            (int *) calloc(NB_HIT_FACES, sizeof(int));
        assert(c->cell_info->faces_hit);
        for (i = 0; i < NB_HIT_FACES; i++) {
            fscanf(fd, "%d", &(c->cell_info->faces_hit[i]));
        }

        /* block hit */
        fscanf(fd, "%d", &(c->cell_info->nblocks));
        if (c->cell_info->nblocks) {
            c->cell_info->block_hit =
                (blocknum_t *) malloc(c->cell_info->nblocks *
                                      sizeof(blocknum_t));
            assert(c->cell_info->block_hit);
        } else {
            c->cell_info->block_hit = NULL;
        }
        for (i = 0; i < c->cell_info->nblocks; i++) {
            fscanf(fd, "%d", &bh);
            c->cell_info->block_hit[i] = (blocknum_t) bh;
        }

        /* items */
        fscanf(fd, "%d", &(c->cell_info->nitems));
        if (!c->cell_info->item) {
            c->cell_info->item = (struct cell_info_item_t *)
                malloc(c->cell_info->nitems *
                       sizeof(struct cell_info_item_t));
            assert(c->cell_info->item);
        } else {
            fprintf(stderr,
                    "Cell item [z=%d,x=%d,y=%d] is already set, exiting !\n",
                    z, x, y);
            exit(1);
        }

        for (i = 0; i < c->cell_info->nitems; i++) {
            c->cell_info->item[i].in =
                (struct coord_geo_t *) malloc(sizeof(struct coord_geo_t));
            assert(c->cell_info->item[i].in);
            c->cell_info->item[i].out =
                (struct coord_geo_t *) malloc(sizeof(struct coord_geo_t));
            assert(c->cell_info->item[i].out);
            fscanf(fd, "%ld %lf %lf %lf %lf %lf %lf %lf %lf",
                   &(c->cell_info->item[i].rayid),
                   &(c->cell_info->item[i].in->lat),
                   &(c->cell_info->item[i].in->lon),
                   &(c->cell_info->item[i].in->prof),
                   &(c->cell_info->item[i].out->lat),
                   &(c->cell_info->item[i].out->lon),
                   &(c->cell_info->item[i].out->prof),
                   &(c->cell_info->item[i].P_length),
                   &(c->cell_info->item[i].S_length));

        }

        /* sort by rayid */
        qsort(c->cell_info->item, c->cell_info->nitems,
              sizeof(struct cell_info_item_t), compare_rayid);

        /* store the highest rayid (gimbos stuff) */
        if (c->cell_info->item[c->cell_info->nitems - 1].rayid >
            mesh->max_rayid) {
            mesh->max_rayid =
                c->cell_info->item[c->cell_info->nitems - 1].rayid;
        }
    }
    fclose(fd);

    /* fprintf(stderr, "mesh->max_rayid = %ld\n", mesh->max_rayid); */
    return (1);
}

/** \brief Import sco data file
 */
int
import2mesh_sco_file(struct mesh_t *mesh, char *filename,
                     int use_log_scale, struct mesh_offset_t **offset)
{
    FILE *fd;

    struct cell_t *c;

    float *score;

    int x, y, z;

    int s;

    char line[256];

    char *val;

    int nb, nbline;

    fprintf(stdout, "Importing sco formated file '%s'\n", filename);
    if (!(fd = fopen(filename, "r"))) {
        perror(filename);
        exit(1);
    }
    /*
     * first line is commented but contains usefull information -
     * deprecated
     */
    fgets(line, 256, fd);

    /* nb score, score name */
    fgets(line, 256, fd);
    val = strtok(line, " \t\n");
    NB_SCORE = atoi(val);
    /* fprintf (stderr, "nb score  = %d\n", NB_SCORE); */
    if (NB_SCORE > NB_SCORE_MAX) {
        fprintf(stderr,
                "mesh : import (%s): nb score max is %d, exiting !\n",
                filename, NB_SCORE);
        exit(1);
    }
    SCORE_NAME = (char **) malloc(NB_SCORE * sizeof(char *));
    score = (float *) calloc(sizeof(float), NB_SCORE);
    assert(score);

    for (s = 0; s < NB_SCORE; s++) {
        SCORE_NAME[s] = strdup(strtok(NULL, " \t\n"));
    }

    nbline = 2;
    while (!feof(fd)) {

        /* read data */
        nbline++;
        nb = fscanf(fd, "%*c%d%*c%d%*c%d%*c", &x, &y, &z);
        if (nb != 3) {
            fprintf(stderr,
                    "Error reading(%d) line %d ([x,y,z]), exiting !\n", nb,
                    nbline);
            exit(1);
        }
        for (s = 0; s < NB_SCORE; s++) {
            nb = fscanf(fd, "%f", score + s);
            if (nb != 1) {
                fprintf(stderr,
                        "Error reading(%d) line %d step %d, exiting !\n",
                        nb, nbline, s);
                exit(1);
            }
        }
        fscanf(fd, "\n");

        /* Apply offset  */
        if (offset && offset[z]) {
            int mod;

            /* usefull for modulo stuff */
            mod =
                (360.0 /
                 (mesh->parameter->lon_unit_size *
                  mesh->layer[z]->lon_unit));

            /* z += offset->z; */
            x += offset[z]->lat;
            y = modulo(y + offset[z]->lon + mod, mod);
        }
        /* check if we are in the mesh bounds */
        if (z > mesh->nlayers) {
            fprintf(stderr,
                    "import error : layer = %d, max layer should be %d, exiting !\n",
                    z, mesh->nlayers);
            exit(1);
        }
        /* check if user want this layer (command option) */
        if (!mesh->layer[z]) {
            continue;
        }
        /* attention au couche NULL */
        if (x < 0 || x >= mesh->layer[z]->nlat) {
            continue;
            /*
             * fprintf(stderr, "mesh : import(line %d): x = %d,
             * max x should be %d\n", nbline, x,
             * mesh->layer[z]->nlat); if (offset[z]) {
             * fprintf(stderr, "offset[lat=%d,lon=%d,prf=%d]\n",
             * offset[z]->lat,offset[z]->lon, offset[z]->z); }
             * fprintf(stderr, "[%d,%d,%d], exiting !\n", x,y,z);
             * exit(1);
             */
        }
        if (y < 0 || y >= mesh->layer[z]->nlon) {
            continue;
            /*
             * fprintf(stderr, "mesh : import(line %d): y = %d,
             * max y should be %d, exiting !\n", nbline, y,
             * mesh->layer[z]->nlon); exit(1);
             */
        }
        /* go the layer where lie the cell */
        c = mesh->layer[z]->cell;

        /* locate the [x=lat,y=lon,z=layer] cell */
        c = move_in_mesh(mesh, c, x, y, z);
        if (!c) {
            fprintf(stderr,
                    "move_in_mesh: error can't go to (x=%d,y=%d,z=%d)\n",
                    x, y, z);
            exit(1);
        }
        /* set the cell id */
        c->id.x = x;
        c->id.y = y;
        c->id.z = z;

        /* alloc */
        if (!c->cell_info) {
            c->cell_info =
                (struct cell_info_t *) calloc(1,
                                              sizeof(struct cell_info_t));
            assert(c->cell_info);
        }
        if (!c->cell_info->score) {
            c->cell_info->score =
                (float *) calloc(NB_SCORE, sizeof(float));
            assert(c->cell_info->score);
        }
        /* store score info in the mesh */
        if (use_log_scale) {
            for (s = 0; s < NB_SCORE; s++) {
                c->cell_info->score[s] = log10f(1 + score[s]);
            }
        } else {
            for (s = 0; s < NB_SCORE; s++) {
                c->cell_info->score[s] = score[s];
            }
        }
    }
    fclose(fd);
    free(score);

    fprintf(stdout, "\t%d non null cells imported\n",
            nbline - 2 /* header */ );
    return (1);
}

/** \brief Import irregular mesh (IRM) information into mesh
 */
int
import2mesh_irm_file(struct mesh_t *mesh,
                     char *filename, struct mesh_offset_t **offset)
{
    int nb_metacells;

    int x, y, z, m, n, i;

    float score;

    char line[256];

    int ret;

    int mm, nn;

    int south, north;

    int nb_cell;

    int layer;

    struct cell_t **headcell_tab;

    struct cell_t *c, *start_cell, *hcell;

    FILE *fd;

    fprintf(stdout, "importing irm formated file '%s' ...", filename);
    if (!(fd = fopen(filename, "r"))) {
        perror(filename);
        exit(1);
    }
    /* ignore commented lines : starting with '#' */
    fgets(line, 1256, fd);
    fgets(line, 1256, fd);
    fgets(line, 1256, fd);

    fscanf(fd, "%d\n", &nb_metacells);

    /* store the link to the heading cell in the mesh */
    headcell_tab = (struct cell_t **)
        calloc(nb_metacells, sizeof(struct cell_t *));
    assert(headcell_tab);

    for (i = 0; i < nb_metacells; i++) {
        ret = fscanf(fd, "%d %d %d", &x, &y, &z);
        if (ret != 3) {
            fprintf(stderr,
                    "%s:import2mesh_irm_file():%d: could only read %d items\n",
                    __FILE__, __LINE__, ret);
            exit(1);
        }
        if (offset && offset[z]) {
            int mod;

            /* usefull for modulo stuff */
            mod =
                (360.0 /
                 (mesh->parameter->lon_unit_size *
                  mesh->layer[z]->lon_unit));

            /* Apply offset  */
            /* z += offset->z; */
            x += offset[z]->lat;
            y = modulo(y + offset[z]->lon + mod, mod);
        }
        if (!mesh->layer[z]) {
            fprintf(stdout, " layer skipped\n");
            return (0);
        }
        /* locate the [x=lat,y=lon,z=layer] cell */
        hcell = move_in_mesh(mesh, NULL, x, y, z);
        if (!hcell) {
            fprintf(stderr,
                    "move_in_mesh: error can't go to (x=%d,y=%d,z=%d)\n",
                    x, y, z);
            exit(1);
        }
        headcell_tab[i] = hcell;

        ret = fscanf(fd, "%d %d %f\n", &m, &n, &score);
        if (ret != 3) {
            fprintf(stderr,
                    "%s: import2mesh_irm_file(): %d: could only read %d items\n",
                    __FILE__, __LINE__, ret);
            exit(1);
        }
        /* update the metacell */
        hcell->nb_meta_scores = 1;
        hcell->meta_scores = (struct meta_score_t *)
            malloc(sizeof(struct meta_score_t));
        hcell->meta_scores[0].m = m;
        hcell->meta_scores[0].n = n;
        hcell->meta_scores[0].score = score;

        /* update all the unit cell in the metacell */
        north = get_north_direction(hcell, mesh->layer[z], mesh);
        south = opposite_dir(north);

        nb_cell = 0;
        c = start_cell = hcell;
        for (nn = 0; nn < n && c; nn++) {
            for (mm = 0; mm < m && c; mm++) {
                nb_cell++;

                /*
                 * checks the cell c is not already
                 * associated with a metacell
                 */
                assert(!c->meta_cell);
                c->meta_cell = hcell;

                /* next cell */
                c = get_cell_from_list(c->neighbour_list[EAST_D], 0);
            }
            c = start_cell =
                get_cell_from_list(start_cell->neighbour_list[south], 0);
        }

        if (nb_cell != m * n) {
            fprintf(stderr,
                    "%s: import2mesh_irm_file(): %d: internal error, meta-cell problem !\n",
                    __FILE__, __LINE__);
            exit(1);
        }
    }

    if (i != nb_metacells) {
        fprintf(stderr, "%s: troncated irm file (%d/%d)\n",
                __FILE__, i, nb_metacells);
        exit(1);
    }

    fclose(fd);

    /* update the mesh with the metacells */
    layer = headcell_tab[0]->id.z;
    mesh->metacell[layer] = headcell_tab;
    mesh->nb_metacell[layer] = nb_metacells;
    mesh->nb_total_metacell += nb_metacells;

    fprintf(stdout, " %d metacells\n", nb_metacells);

    return (nb_metacells);
}
