#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <popt.h>

#include "mesh.h"
#include "mesh2xml.h"

/*--------------------*/

/* parse_command_line */

/*--------------------*/
void
parse_command_line(int argc,
                   char **argv,
                   char **meshfilename,
                   char **basename, int *lat_div, int *lon_div)
{
    static int lat_div_opt = 1;

    static int lon_div_opt = 1;

    static char *mopt = NULL;

    static char *bname = NULL;

/* popt */
    poptContext cx;

    int rc;

#include "mesh_split_options.h"

    /* next 5 lines are a horrible hack for the
       code to compile on IRIX : normally, we would not need
       the intermediate var theta_opt,length_opt, and subsample_opt.
       More over, they must be declared as static 
       the cc compiler to accept them as popt variables */

    /* args parsing using popt */
    cx = poptGetContext(PACKAGE, argc, (const char **) argv, split_options,
                        0);
    /*poptAddAlias(cx, alias, 0); */

    do {
        rc = poptGetNextOpt(cx);
        switch (rc) {
            /* help */
        case 1:
            poptPrintHelp(cx, stdout, 0);
            exit(0);
            break;
        case 10:
            fprintf(stdout, "%s-%s\n", PACKAGE, VERSION);
            exit(0);
            break;
        case POPT_ERROR_BADOPT:        /* error */
            fprintf(stderr, "%s: %s\n", poptBadOption(cx, 0),
                    poptStrerror(rc));
            poptPrintUsage(cx, stdout, 0);
            exit(1);
        }
    }
    while (rc > 0);

    *lat_div = lat_div_opt;
    *lon_div = lon_div_opt;
    *meshfilename = mopt;
    *basename = bname;

    poptFreeContext(cx);
}

/*--------*/

/*  main  */

/*--------*/
int main(int argc, char **argv)
{
    struct mesh_t *mesh;

    float lon_part, lat_part;

    int lat_div, lon_div, i, j;

    float lon_min_orig, lat_min_orig;

    char *meshfile, *basename = NULL, filename[256];

    parse_command_line(argc, argv, &meshfile, &basename, &lat_div,
                       &lon_div);

    if (!basename) {
        fprintf(stderr, "missing -b option\n");
        exit(1);
    }

    if (meshfile) {
        if (access(meshfile, R_OK)) {
            perror(meshfile);
            exit(1);
        }
    } else {
        fprintf(stderr, "missing -m option\n");
        exit(1);
    }
    if (!(mesh = mesh_init_from_file(meshfile))) {
        printf("Could not parse mesh file %s\n", meshfile);
        exit(1);
    }

    /* split longitude into regular angles */
    lon_part =
        fabs(mesh->parameter->lon_max -
             mesh->parameter->lon_min) / lon_div;
    lat_part =
        fabs(mesh->parameter->lat_max -
             mesh->parameter->lat_min) / lat_div;

    /* store original min boundaries */
    lon_min_orig = mesh->parameter->lon_min;
    lat_min_orig = mesh->parameter->lat_min;

    for (i = 0; i < lat_div; i++) {
        mesh->parameter->lat_min = i * lat_part + lat_min_orig;
        mesh->parameter->lat_max = mesh->parameter->lat_min + lat_part;
        for (j = 0; j < lon_div; j++) {
            mesh->parameter->lon_min = j * lon_part + lon_min_orig;
            mesh->parameter->lon_max = mesh->parameter->lon_min + lon_part;

            /* new file */
            sprintf(filename, "%s-%d-%d.xml", basename, i, j);
            mesh2xml(mesh, filename);
        }
    }

    return (0);
}
