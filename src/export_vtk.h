#include <stdio.h>
#include <string.h>

#include "point3d.h"
#include "cell.h"
#include "mesh.h"
#include "layer.h"

#ifndef __EXPORT_VTK_H__
#define __EXPORT_VTK_H__

void dump_vtk_point3d(FILE * fd, struct point3d_tab_t *tpi,
                      char *coordtype);
void dump_vtk_cell(FILE * fd, struct cell_tab_t *tc, int with_score);

void dump_vtk_metacell(FILE * fd, struct mesh_t *mesh);

void vtk_header_polydata(FILE * fd, char *texte);

void vtk_header_unstructured_grid(FILE * fd, char *texte);

void dump_vtk_point3d_value(FILE * fd, struct point3d_tab_t *tabpoint,
                            int nb);
void make_vtk_lut(FILE * fd, int nbval);

void make_vtk_lut_for_layer(FILE * fd, struct mesh_t *mesh);

void dump_vtk_slice(FILE * fd, struct point3d_tab_t *tabpoint,
                    struct cell_t *c_ini, int nbcell, int sens,
                    int nblayer);
#endif
