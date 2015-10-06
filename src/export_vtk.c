#include "cellinfo.h"
#include "metacell.h"
#include "export_vtk.h"
#include "extern.h"

void vtk_header_polydata(FILE * fd, char *texte)
{
    fprintf(fd, "# vtk DataFile Version 2.0\n");
    fprintf(fd, "%s\n", texte);
    fprintf(fd, "ASCII\n");
    fprintf(fd, "DATASET POLYDATA\n\n");
}

void vtk_header_unstructured_grid(FILE * fd, char *texte)
{
    fprintf(fd, "# vtk DataFile Version 2.0\n");
    fprintf(fd, "%s\n", texte);
    fprintf(fd, "ASCII\n");
    fprintf(fd, "DATASET UNSTRUCTURED_GRID\n\n");
}

void dump_vtk_point3d(FILE * fd,
                      struct point3d_tab_t *tabpoint, char *coordtype)
{
    register int i;

    double r, cos_lat, sin_lat, cos_lon, sin_lon;

    double x, y, z;

    fprintf(fd, "POINTS %d float\n", tabpoint->nbpoint);
    for (i = 0; i < tabpoint->nbpoint; i++) {

        if (strstr(coordtype, "SPHERIQUE") != NULL) {
            fprintf(fd, "%f %f %f\n", tabpoint->tab[i]->lat,
                    tabpoint->tab[i]->lon, tabpoint->tab[i]->prf);
            continue;
        }

        /*geo2cart */

        r = RE - tabpoint->tab[i]->prf;
        cos_lat = cos(tabpoint->tab[i]->lat * TO_RAD);
        sin_lat = sin(tabpoint->tab[i]->lat * TO_RAD);
        cos_lon = cos(tabpoint->tab[i]->lon * TO_RAD);
        sin_lon = sin(tabpoint->tab[i]->lon * TO_RAD);

        x = r * cos_lat * cos_lon;
        y = r * cos_lat * sin_lon;
        z = r * sin_lat;

        fprintf(fd, "%f %f %f\n", x, y, z);
    }
    fprintf(fd, "\n");
}

void dump_vtk_metacell(FILE * fd, struct mesh_t *mesh)
{

    int nb_total_metacells = 0;

    int nb_metacells = 0;

    int l, i, j;

    int points_id[8];

    for (l = 0; l < mesh->nlayers; l++) {
        if (!mesh->layer[l])
            continue;
        nb_total_metacells += mesh->nb_metacell[l];
    }

    fprintf(fd, "CELLS %d %d\n",
            nb_total_metacells,
            nb_total_metacells * 8 + nb_total_metacells);

    for (l = 0; l < mesh->nlayers; l++) {
        if (!mesh->layer[l])
            continue;
        nb_metacells = mesh->nb_metacell[l];
        for (i = 0; i < nb_metacells; i++) {
            metacell_get_points_id(mesh, mesh->metacell[l][i], points_id);
            fprintf(fd, "8 ");
            for (j = 0; j < 8; j++) {
                /* point number */
                fprintf(fd, "%d ", points_id[j]);
            }
            fprintf(fd, "\n");
        }
    }

    fprintf(fd, "\nCELL_TYPES %d\n", nb_total_metacells);
    for (i = 0; i < nb_total_metacells; i++) {
        fprintf(fd, "12\n");
    }

    fprintf(fd, "\nCELL_DATA %d\n", nb_total_metacells);
    fprintf(fd, "SCALARS illumination float 1\nLOOKUP_TABLE default\n");
    for (l = 0; l < mesh->nlayers; l++) {
        if (!mesh->layer[l])
            continue;
        nb_metacells = mesh->nb_metacell[l];
        for (i = 0; i < nb_metacells; i++) {
            fprintf(fd, "%f\n",
                    mesh->metacell[l][i]->meta_scores[0].score);
        }
    }

}

void dump_vtk_cell(FILE * fd, struct cell_tab_t *tabcell, int with_score)
{
    register int i, j, s;

    fprintf(fd, "CELLS %d %d\n", tabcell->nbcell,
            tabcell->nbcell * 8 + tabcell->nbcell);
    for (i = 0; i < tabcell->nbcell; i++) {

        if (tabcell->tab[i]->point[0] == -1) {
            fprintf(fd, "8 0 0 0 0 0 0 0 0\n");
            continue;
        }

        fprintf(fd, "8 ");
        for (j = 0; j < 8; j++) {
            /* point number */
            fprintf(fd, "%d ", tabcell->tab[i]->point[j]);
        }
        fprintf(fd, "\n");
    }

    fprintf(fd, "\nCELL_TYPES %d\n", tabcell->nbcell);
    for (i = 0; i < tabcell->nbcell; i++) {
        fprintf(fd, "12\n");
    }

    /* no score in  mesh */
    if (!with_score) {
        /*fprintf(fd, "\nCELL_DATA %d\n", tabcell->nbcell);
           fprintf(fd, "SCALARS cell_scalars int 1\nLOOKUP_TABLE default\n");
           for (i = 0; i < tabcell->nbcell; i++) {
           fprintf(fd, "%d\n", i);
           } */
        return;
    }

    /* dump each cell value/score separatly */
    /* only if import file was specified */
    if (NB_SCORE == 0)
        return;

    fprintf(fd, "\nCELL_DATA %d\n", tabcell->nbcell);
    for (s = 0; s < NB_SCORE; s++) {
        fprintf(fd, "SCALARS %s float 1\nLOOKUP_TABLE default\n",
                SCORE_NAME[s]);
        for (i = 0; i < tabcell->nbcell; i++) {
            if (tabcell->tab[i]->cell_info) {
                fprintf(fd, "%f\n", tabcell->tab[i]->cell_info->score[s]);
            } else {
                fprintf(fd, "%f\n", 0.0);
            }
        }
    }
}

void
dump_vtk_point3d_value(FILE * fd, struct point3d_tab_t *tabpoint, int nb)
{
    register int i;

    int col = 0;

    fprintf(fd, "\nPOINT_DATA %d\n", tabpoint->nbpoint);
    fprintf(fd, "SCALARS point_scalars int 1\nLOOKUP_TABLE default\n");
    for (i = 0; i < tabpoint->nbpoint; i++) {
        fprintf(fd, "%d\n", col);
        if ((i + 1) % nb == 0)
            col++;
    }
}

void make_vtk_lut_for_layer(FILE * fd, struct mesh_t *mesh)
{
    int l, nb;

    fprintf(fd, "\nCELL_DATA %ld\n", mesh->ncells);
    fprintf(fd, "SCALARS layer int 1\n");
    fprintf(fd, "LOOKUP_TABLE default\n");
    for (l = 0; l < mesh->nlayers; l++) {
        for (nb = 0; mesh->layer[l] && nb < mesh->layer[l]->ncells; nb++) {
            /* vtk lookupup table don't like 0 data ! */
            fprintf(fd, "%d ", l + 1);
        }
        fprintf(fd, "\n");
    }
    fprintf(fd, "\n");
}

void make_vtk_lut(FILE * fd, int nbval)
{
    int i, n;

    float blue, red, green, alpha;

    fprintf(fd, "\nLOOKUP_TABLE TEST %d\n", nbval);
    alpha = 1.;
    n = 0;
    for (i = 0; i < 256 && n < nbval; i += (int) (256 / nbval)) {
        blue = (float) i / 255;
        red = 1. - blue;
        green = 0;
        /*fprintf (fd, "%.1f %.1f %.1f %.1f\n", red, green, blue, alpha); */
        fprintf(fd, "1 0 0 1\n");
        n++;
    }
}

/* create a slice from a mesh, 
 * given the direction, and the number of cell in that direction (-1 for all cells) */
void
dump_vtk_slice(FILE * fd, struct point3d_tab_t *tabpoint,
               struct cell_t *c_ini, int nbcell, int sens, int nblayer)
{
    struct cell_t *c, *cl;
    struct cell_tab_t tabcell = { NULL, 0, 100 };
    int n = 0, nl = 0;

    int i, j;

    c = c_ini;

    /* nb max de cellules */
    do {
        n++;
        c = get_cell_from_list(c->neighbour_list[sens], 0);
    }
    while (c != c_ini && c != NULL);

    if (nbcell > 0) {
        if (nbcell < n)
            n = nbcell;
    }
    fprintf(stderr, "dump_vtk_slice : nb cells = %d\n", n);

    /* nb max layers */
    c = c_ini;
    nl = 1;
    while (c->neighbour_list[DOWN_D] != NULL) {
        c = get_cell_from_list(c->neighbour_list[DOWN_D], 0);
        nl++;
    }
    if (nblayer > 0) {
        if (nblayer < nl)
            nl = nblayer;
    }
    fprintf(stderr, "dump_vtk_slice : nb layers = %d\n", nl);

    cl = c_ini;
    j = 0;
    while (j < nl) {
        i = 0;
        c = cl;
        while (i < n) {
            add_cell_to_tab(&tabcell, c);
            c = get_cell_from_list(c->neighbour_list[sens], 0);
            i++;
        }
        cl = get_cell_from_list(cl->neighbour_list[DOWN_D], 0);
        j++;
    }

    vtk_header_unstructured_grid(fd, "maillage -- slice");
    dump_vtk_point3d(fd, tabpoint, "CARTESIEN");
    dump_vtk_cell(fd, &tabcell, 0);
    dump_vtk_point3d_value(fd, tabpoint, n);
    make_vtk_lut(fd, n);

}
