#ifndef __MERIDIAN_H__
#define __MERIDIAN_H__

struct cell_t *make_meridian(struct layer_t *layer, struct mesh_t *mesh);

int link_meridian_1(struct cell_t *c1, struct cell_t *c2);

int link_meridian_2(struct cell_t *c1, struct cell_t *c2);

int link_meridian_3(struct cell_t *c1, struct cell_t *c2);
#endif
