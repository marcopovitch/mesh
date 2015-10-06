#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <math.h>

#include "mesh.h"
#include "layer.h"
#include "extern.h"

#ifndef __MESH2XML_H__
#define __MESH2XML_H__

void parameter2xml(FILE * fd, struct mesh_parameter_t *mp);

void section_data2xml(FILE * fd, struct mesh_data_t *md);

void mesh2xml(struct mesh_t *mesh, char *filename);

#endif
