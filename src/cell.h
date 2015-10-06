#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>

#ifndef __CELL_H__
#define __CELL_H__

#include "point3d.h"

/*
 *                         DATA STRUCTURES
 *
 */

/* enum { NSD_1, NSD_2, EAST, WEST, UP, DOWN }; */

/* static const char *direction_name[6]={"Nort->South_1","Nort->South_2","->East","->West","->Up","->Down"}; */

#define NSD_1 0                 /**< North South direction 1 */
#define NSD_2 1                 /**< North South direction 2 */
#define EAST_D  2               /**< EAST direction */
#define WEST_D  3               /**< WEST direction */
#define UP_D    4               /**< UP direction */
#define DOWN_D  5               /**< DOWN direction */

/** \brief cell coordinates in the mesh */
struct coord_z3_t {
    int x;              /**< cell lat coord */
    int y;              /**< cell lon coord */
    int z;              /**< cell layer     */
};

/** \brief keep score information for a meta-cell made of
 *  (m x n) cells 
 */
struct meta_score_t {
    float score;                        /**< meta-cell score after comparison func */
    float real_score;                   /**< the real score obtains in a meta-cell */
    int m;                              /**< nb of basic cell in lon */
    int n;                              /**< nb of basic cell in lat */
};

/** \brief cell definition */
struct cell_t {
    struct coord_z3_t id;                  /**< cell coordinates in the mesh */
    unsigned int point[8];                 /**< array of indexed points defined  */
    struct cell_list_t *neighbour_list[6]; /**< cell's neighbourhood */
    struct cell_info_t *cell_info;         /**< score information summary */

    /* iregular mesh info */
    struct cell_t *meta_cell;              /**< link to its heading cell in mesh */
    struct cell_list_t **meta_neighbour_list;    /**< metacell's neighbourhood */

    /* gimbos specific stuff */
    int nb_meta_scores;                    /**< nb of element in meta_scores array */
    struct meta_score_t *meta_scores;      /**< array of meta-cell's score which have 
				                coord_z3_t id as the heading cell */

    /* gimbos internal use */
    struct cell_list_t *list_pos;          /**< pointer on list L item */
    char selected;                         /**< true if the cell was selected by gimbos */
};

/** \brief linked cell list */
struct cell_list_t {
    struct cell_t *cell;        /**< current cell */
    struct cell_list_t *next;   /**< next cell in the list */
    struct cell_list_t *pred;   /**< predecessor cell in the list */
    int array_pos;              /**< array position (Gimbos) */
};

/** \brief pointer cells array */
struct cell_tab_t {
    struct cell_t **tab;        /**< cell array */
    unsigned int nbcell;        /**< nb of cell in this array */
    unsigned int blockminsize;  /**< new block allocation size */
};

/*
 * ----------------------------------------------------------------
 *
 *                        METHODS 
 *
 * ----------------------------------------------------------------
 */

void dump_cell(struct point3d_tab_t *tp, struct cell_t *c, char *s);

struct cell_t *new_cell(struct cell_tab_t *tabcell,
                        const struct coord_z3_t *cell_id);

void destroy_cell(struct cell_t *c);

struct cell_t *get_cell_from_list(struct cell_list_t *cl, int indice);

void dump_cell_list(struct cell_list_t *cl);

void destroy_cell_list(struct cell_list_t *l);

int cell_list_get_nb_item(struct cell_list_t *l);

void updateinfo_cell(struct cell_t *cell, float lat, float lon,
                     float prf, float plat, float plon, float pprf,
                     struct point3d_tab_t *point3d_tab);

void create_points_for_cell(struct cell_t *cell, float lat, float lon,
                            float prf, float plat, float plon, float pprf,
                            int *point2create,
                            struct point3d_tab_t *point3d_tab,
                            char *texte);
void connect_cell(struct cell_t *c1, struct cell_t *c2, int *pt1,
                  int *pt2);
void set_point2create(int *pc, int *face, int val);

int edge_cell_without_point(struct cell_t *cell, int *face);

int all_points_set(int *p2c);

void dump_point2create(int *pc);

struct cell_list_t *add_cell_to_list(struct cell_list_t *l,
                                     struct cell_t *c);
struct cell_list_t *remove_cell_from_list(struct cell_list_t *l,
                                          struct cell_t *c);
struct cell_list_t *add_neighbour_to_cell(struct cell_t *c1,
                                          struct cell_t *c2, int dir);
void permut_cell_list(struct cell_t ***t1, struct cell_t ***t2);

struct cell_tab_t *add_cell_to_tab(struct cell_tab_t *t, struct cell_t *c);

struct cell_t *cell_move(struct cell_t *c_ini, int sens, int pas);

int opposite_dir(int dir);

struct point3d_t *get_face_vertices(struct cell_t *cell, int face,
                                    struct point3d_tab_t *point3d_tab);

#endif
