#ifndef _MESH_EXTERN_H_
#define _MESH_EXTERN_H_

#include "point3d.h"
#include "cell.h"

extern const char *DIRECTION[];

extern const int NLAYERS;

extern const float INTERFACES_BOUNDS[];

extern int VERBOSE;

extern int DEBUG;

extern int ALLOC_ALL_POINTS;

extern int DO_NOT_LINK_LAYERS;

extern int NB_SCORE;

extern char **SCORE_NAME;

extern const int NB_MESH_FILE_FORMAT;

extern const char *MESH_FILE_FORMAT[];

#endif
