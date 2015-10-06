#ifndef __MESH_METACELL__
#define __MESH_METACELL__

void metacell_find_neighbourhood(struct mesh_t *mesh);

void metacell_get_points_id(struct mesh_t *mesh,
                            struct cell_t *head_cell, int *point_id);

#endif
