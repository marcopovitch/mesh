#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "export_r2m.h"
#include "extern.h"

void export_r2m(struct mesh_t *m, struct cell_tab_t *tabcell,
                char *filename)
{
    int i, j;

    int nb_cell = 0;

    FILE *fd;

    struct cell_info_t *ci;

    fd = fopen(filename, "w");

    fprintf(fd, "# format=r2m, generated by %s v%s, mesh=%s\n",
            PACKAGE, VERSION, m->xml_filename);

    for (j = 0; j < tabcell->nbcell; j++) {
        if (tabcell->tab[j]->cell_info) {
            nb_cell++;
        }
    }
    fprintf(fd, "%d\n", nb_cell);

    if (nb_cell != 0) {
        for (j = 0; j < tabcell->nbcell; j++) {
            ci = tabcell->tab[j]->cell_info;
            /* allocated cells with nitems=0 are orphan cells : ignored */
            if (ci == NULL)
                continue;
            if (ci->nitems == 0)
                continue;

            /* x y z */
            fprintf(fd, "%d %d %d\n",
                    tabcell->tab[j]->id.z,
                    tabcell->tab[j]->id.x, tabcell->tab[j]->id.y);

            /* face hit */
            for (i = 0; i < NB_HIT_FACES; i++)
                fprintf(fd, "%d ", ci->faces_hit[i]);
            fprintf(fd, "\n");

            /* block hit */
            fprintf(fd, "%d ", ci->nblocks);
            for (i = 0; i < ci->nblocks; i++)
                fprintf(fd, "%d ", ci->block_hit[i]);
            fprintf(fd, "\n");

            /* items */
            fprintf(fd, "%d\n", ci->nitems);
            for (i = 0; i < ci->nitems; i++) {
                fprintf(fd, "%ld %f %f %f %f %f %f %f %f\n",
                        ci->item[i].rayid,
                        ci->item[i].in->lat,
                        ci->item[i].in->lon,
                        ci->item[i].in->prof,
                        ci->item[i].out->lat,
                        ci->item[i].out->lon,
                        ci->item[i].out->prof,
                        ci->item[i].P_length, ci->item[i].S_length);
            }
            fprintf(fd, "\n");

        }                       /* for */
    }
    /* if */
    fprintf(fd, "# end\n");
    fclose(fd);
}
