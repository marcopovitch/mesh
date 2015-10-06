#ifndef __FUSION_H__
#define __FUSION_H__

int **fusion_matrix_alloc(int l, int c);

void feed_fusion_matrix(int **F, int score_rank,
                        struct layer_t *layer,
                        struct mesh_parameter_t *mp);

#endif
