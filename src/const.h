#ifndef __MESH_CONST_H__
#define __MESH_CONST_H__

#include <math.h>

#define RE 6371.0
#define TO_DEG  180.0/M_PI
#define TO_RAD 	M_PI/180.0

#ifndef EPS
#define EPS     0.001
#endif

enum { R2M = 0, SCO, RES, SPARSE, EVT, IRM };

#endif
