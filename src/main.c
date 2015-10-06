
/*
 * Ray2mesh : software for geophysicists.
 * Compute various scores attached to the mesh cells, based on geometric
 * information that rays bring when they traverse cells.
 *
 * Copyright (C) 2003, Stéphane Genaud and Marc Grunberg
 *
 * This tool is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 **/

/* $Id: main.c,v 1.136 2007-05-17 12:08:17 marc Exp $ */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

#ifdef __APPLE__
#include <malloc/malloc.h>
#else
#include <malloc.h>
#endif

#include <popt.h>
#include <locale.h>
#include <libxml/tree.h>        /* xml (libxml-2) */
#include <libxml/parser.h>      /* xml */

#include "const.h"
#include "point3d.h"
#include "cell.h"
#include "mesh.h"
#include "layer.h"
#include "export_vtk.h"
#include "import.h"
#include "extern.h"

/** \brief show memory used RSIZE, VSIZE */
void show_mem_usage()
{
#ifdef __APPLE__
    /*
     * struct mstats m; m = mstats();
     * 
     * malloc_statistics_t memusage; malloc_zone_statistics(NULL,
     * &memusage);
     * 
     * fprintf(stderr, "Memory used: %.2f MBytes, mstat=%2.f MBytes\n",
     * (float)(memusage.size_in_use)/(1024.*1024.), m.bytes_total);
     */

    task_basic_info_data_t ti;

    mach_msg_type_number_t count;

    vm_statistics_data_t vm_info;

    vm_size_t vsize, rsize;

    task_t a_task = mach_task_self();

    count = TASK_BASIC_INFO_COUNT;
    task_info(a_task, TASK_BASIC_INFO, (task_info_t) & ti, &count);
    rsize = ti.resident_size;
    vsize = ti.virtual_size;

    fprintf(stderr, "Memory used: RSIZE=%.2f VSIZE=%.2f MBytes\n",
            rsize / (1024. * 1024.), vsize / (1024. * 1024.));
#else
    {
        struct mallinfo m_info;

        m_info = mallinfo();
        fprintf(stderr, "Memory used: %.2f MBytes\n",
                (float) (m_info.uordblks +
                         m_info.usmblks) / (1024. * 1024.));
    }
#endif
}

/** \brief mesh_info : mesh lib version (internal use)
 *
 * This one should be used internally : calls the public libmeshversion()
 * method to retrieve library version used.
 **/
static void mesh_info(void)
{
    char *libver = libmeshversion();

    fprintf(stdout, "mesh-%s: using library version %s\n", VERSION,
            libver);
    free(libver);
}

/*-------------------------------------------------------------------------*/

/* main()                                                                  */

/*-------------------------------------------------------------------------*/
int main(int argc, char **argv)
{

    /*
     * remark : for poptOptions to compile on Irix, make the
     * assigned variable static 
     */
    int i;
    int layer;
    static int begin_layer = -1;
    static int end_layer = -1;
    int use_log_scale = 0;
    FILE *fd;
    int des;
    char *old_locale;           /* force a dot formatting for floats
                                 * (instead of ,) */
    static char *vtkfilename = NULL;
    int rc;
    poptContext cx;
    static int show_xml_section = 0;
    static char *conffilename = NULL;
    char **xmlfilelist = NULL;
    int nb_xmlfile = 0;
    static char *importfilename = NULL;
    char *data_file;
    static char *format_opt = "sco";
    /* meshes */
    struct mesh_t *mesh;        /* main mesh */
    struct mesh_t **imported_mesh = NULL;       /* meshes to import */
    int format = SCO;           /* default importation format */

#include "options.h"

    /* internationalization */
    /*
     * ... removed for a while bindtextdomain(PACKAGE, LOCALEDIR);
     * textdomain(PACKAGE);
     */

    /***************************/
    /* args parsing using popt */
    /***************************/
    cx = poptGetContext(PACKAGE, argc, (const char **) argv, options, 0);
    do {
        rc = poptGetNextOpt(cx);
        switch (rc) {
            /* help */
        case OPT_FORMAT:
            if (!strcmp(format_opt, "sco")) {
                format = SCO;
                break;
            }
            if (!strcmp(format_opt, "r2m")) {
                format = R2M;
                break;
            }
            if (!strcmp(format_opt, "irm")) {
                format = IRM;
                break;
            }
            fprintf(stderr,
                    "Invalid argument (%s) for -f option, use 'sco', 'r2m' or 'irm'\n",
                    format_opt);
            exit(0);
        case OPT_HELP:
            mesh_info();
            poptPrintHelp(cx, stdout, 0);
            exit(0);
        case OPT_SHOW_XML_SECTION:
            show_xml_section = 1;
            break;
        case OPT_VTK_ALLPOINTS:
            ALLOC_ALL_POINTS = 1;
            break;
        case OPT_USE_LOGSCALE:
            use_log_scale = 1;
            break;
        case OPT_VERSION:
            mesh_info();
            exit(0);
        case POPT_ERROR_BADOPT:        /* error */
            mesh_info();
            fprintf(stderr, "%s: %s\n", poptBadOption(cx, 0),
                    poptStrerror(rc));
            poptPrintUsage(cx, stdout, 0);
            exit(1);
        }
    }
    while (rc > 0);
    poptFreeContext(cx);

    /* vtk output only when importing sco file */
    if (vtkfilename && format == R2M) {
        fprintf(stderr,
                "vtk output only available when importing sco and irm file(s) !\n");
        exit(1);
    }
    /* check confilename */
    if (!conffilename) {
        mesh_info();
        fprintf(stderr, "missing -m option\n");
        exit(1);
    }
    if (access(conffilename, R_OK)) {
        perror(conffilename);
        exit(1);
    }
    /* test if we can open file to import */
    if (importfilename) {
        xmlfilelist = parse_separated_list(importfilename, ",");
        nb_xmlfile = 0;
        while (xmlfilelist[nb_xmlfile]) {
            if (access(xmlfilelist[nb_xmlfile], R_OK) == -1) {
                perror("import file(s)");
                exit(1);
            }
            nb_xmlfile++;
        }
    }
    /* open if require the vtkfile output */
    if (vtkfilename != NULL) {
        if ((des =
             open(vtkfilename, O_WRONLY | O_CREAT | O_EXCL, 0644)) < 0) {
            perror(vtkfilename);
            exit(1);
        }
        fd = fdopen(des, "w");
    } else {
        fd = stderr;
    }

    /****************************/
    /* main mesh initialization */
    /****************************/

    /* inits mesh layers and their params, possibly with a config file */
    if (VERBOSE)
        fprintf(stderr, "(%s):main: parsing \"%s\" config. file...\n",
                __FILE__, conffilename);
    if (!(mesh = mesh_init_from_file(conffilename))) {
        exit(1);
    }
    if (show_xml_section) {
        mesh_show_sections(mesh);
    }
    /* user only want to process layer = [begin_layer, end_layer] */
    select_mesh_layer(mesh, &begin_layer, &end_layer);

    /*************************************************/
    /* initialize slice xml files (with no overlap !) */
    /*************************************************/
    if (nb_xmlfile) {
        int nb_r2m = 0;
        int nb_sco = 0;
        int nb_irm = 0;

        imported_mesh = (struct mesh_t **)
            malloc(sizeof(struct mesh_t *) * nb_xmlfile);

        for (i = 0; i < nb_xmlfile; i++) {
            imported_mesh[i] = mesh_init_from_file(xmlfilelist[i]);

            if (imported_mesh[i]->data[R2M]) {
                nb_r2m += imported_mesh[i]->data[R2M]->ndatafile;
            }
            if (imported_mesh[i]->data[SCO]) {
                nb_sco += imported_mesh[i]->data[SCO]->ndatafile;
            }
            if (imported_mesh[i]->data[IRM]) {
                nb_irm += imported_mesh[i]->data[IRM]->ndatafile;
            }
        }

        if ((format == R2M && nb_r2m == 0) ||
            (format == SCO && nb_sco == 0) ||
            (format == IRM && nb_irm == 0)) {
            fprintf(stderr,
                    "Warning no %s file(s) found in XML file(s)!\n",
                    MESH_FILE_FORMAT[format]);
            fprintf(stderr, "Can't import %s section, exiting !\n",
                    MESH_FILE_FORMAT[format]);
            exit(1);
        }
    }

    /*********************/
    /* mesh construction */
    /*********************/

    /* now, let's start meshing */
    if (VERBOSE) {
        fprintf(stderr, "\n** Meshing layers [%d] -> [%d] **\n\n",
                begin_layer, end_layer);
        mesh_dump_parameter(stderr, "", "", mesh->parameter);
    }
    make_mesh(mesh);

    /* show layer info */
    if (VERBOSE) {
        for (layer = begin_layer; layer <= end_layer; layer++) {
            layer_info("LAYER\n", mesh->layer[layer]);
        }
    }
    fprintf(stderr, "Nb points = %ld, Nb cells = %ld\n",
            mesh->layer[end_layer]->npoints, mesh->ncells);

    if (DEBUG) {
        mesh_dump(mesh);
    }

    /***************************/
    /* slice importation stuff */
    /***************************/
    for (i = 0; i < nb_xmlfile; i++) {
        int j, l;
        struct mesh_offset_t **offset;

        offset = compute_mesh_offset(mesh, imported_mesh[i]);

        for (l = 0; l < mesh->nlayers; l++) {
            if (!offset[l])
                continue;
            fprintf(stderr,
                    "\t%s, [%s] offset[layer=%d] : lat=%d lon=%d z=%d\n",
                    xmlfilelist[i], MESH_FILE_FORMAT[format], l,
                    offset[l]->lat, offset[l]->lon, offset[l]->z);
        }

        for (j = 0; j < imported_mesh[i]->data[format]->ndatafile; j++) {
            data_file = imported_mesh[i]->data[format]->filename[j];
            if (format == R2M) {
                import2mesh_r2m_file(mesh, data_file, offset);
            } else if (format == SCO) {
                import2mesh_sco_file(mesh, data_file, use_log_scale,
                                     offset);
            } else {
                import2mesh_irm_file(mesh, data_file, offset);
            }
        }

        for (l = 0; l < mesh->nlayers; l++) {
            if (offset[l])
                free(offset[l]);
        }
        free(offset);
        free(imported_mesh[i]);
    }
    free(imported_mesh);

    /********************/
    /* save in VTK file */
    /********************/
    if (vtkfilename) {
        /* force floats formatting with a dot separator */
        old_locale = strdup(setlocale(LC_NUMERIC, NULL));
        setlocale(LC_NUMERIC, "C");

        /* dump in vtk file if -o was specified */
        fprintf(stderr, "** Saving to %s file **\n", vtkfilename);
        vtk_header_unstructured_grid(fd, "Maillage");
        fprintf(stderr, "** dumping 3d points to file\n");
        dump_vtk_point3d(fd, mesh->allocated_points, "CARTESIEN");

        if (format == IRM) {
            fprintf(stderr, "** dumping metacells to file\n");
            dump_vtk_metacell(fd, mesh);
        } else {
            /* format == SCO */
            fprintf(stderr, "** dumping cells to file\n");
            if (importfilename) {
                /* with score */
                dump_vtk_cell(fd, mesh->allocated_cells, 1);
            } else {
                /* without score */
                dump_vtk_cell(fd, mesh->allocated_cells, 0);
                make_vtk_lut_for_layer(fd, mesh);
            }
        }
        fclose(fd);

        setlocale(LC_NUMERIC, old_locale);
        free(old_locale);
    }

    /* memory used for the mesh construction */
    show_mem_usage();

    /******************/
    /* Free all stuff */
    /******************/

    /* free points */
    /* free cells */

    /* free mesh */
    free_mesh(mesh);
    mesh = NULL;

    /* free score name */
    for (i = 0; i < NB_SCORE; i++) {
        free(SCORE_NAME[i]);
    }
    free(SCORE_NAME);
    SCORE_NAME = NULL;

    return (0);
}
