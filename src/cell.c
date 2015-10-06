#include "cell.h"
#include "cellinfo.h"
#include "modulo.h"
#include "extern.h"

/** \brief permut the two given cell list t1 and t2 
 **/
void permut_cell_list(struct cell_t ***t1, struct cell_t ***t2)
{
    struct cell_t **tmp;

    tmp = *t1;
    *t1 = *t2;
    *t2 = tmp;
}

/**
 * \brief creates a new cell with corresponding structures 
 **/
struct cell_t *new_cell(struct cell_tab_t *tabcell,
                        const struct coord_z3_t *cell_id)
{
    struct cell_t *cell;

    int i;

    cell = (struct cell_t *) malloc(sizeof(struct cell_t));
    assert(cell);

    /* attention si tabcell change !!!! (realloc) */
    if (tabcell != NULL)
        tabcell = add_cell_to_tab(tabcell, cell);

    for (i = 0; i < 8; i++)
        cell->point[i] = -1;
    for (i = 0; i < 6; i++)
        cell->neighbour_list[i] = NULL;

    cell->cell_info = NULL;

    cell->id.x = cell_id->x;
    cell->id.y = cell_id->y;
    cell->id.z = cell_id->z;

    /* only used by gimbos or lsqrsolve */
    cell->meta_cell = NULL;
    cell->nb_meta_scores = 0;
    cell->meta_neighbour_list = 0;
    cell->meta_scores = NULL;
    cell->selected = 0;

    return (cell);
}

/** \brief  destroy a cell */
void destroy_cell(struct cell_t *c)
{
    int i;

    for (i = 0; i < 4; i++) {
        destroy_cell_list(c->neighbour_list[i]);
    }

    free(c);
}

/** \brief  add a cell to  a given list 
 *
 *  Add a cell to a double linked cell list.
 *  If the list cell_list does not exist, it will
 *  be created, otherwise the cell is added at the tail of the list.
 *  The cell_list is null terminated.
 *  The return value is the head of the list.
 *  There is no doubled item.
 **/

struct cell_list_t *add_cell_to_list(struct cell_list_t *cell_list,
                                     struct cell_t *cell)
{

/*	int nb_cell = 0; */
    struct cell_list_t *ini;

    struct cell_list_t *pred;

    if (!cell_list) {
        /* first cell */
        ini = cell_list =
            (struct cell_list_t *) malloc(sizeof(struct cell_list_t));
        assert(ini);
        if (!cell_list) {
            perror("add_cell_to_list");
            exit(1);
        }
        pred = NULL;
    } else {
        ini = cell_list;
        if (ini->cell == cell) {
            /*fprintf(stdout, "cell(%p) already there\n", cell); */
            return (ini);
        }

        while (cell_list->next) {
            if (cell_list->next->cell == cell) {
                /* doublon */
                /*fprintf(stdout, "cell(%p) already there\n", cell); */
                return (ini);
            }
            cell_list = cell_list->next;
            /* nb_cell++;   */
        }
        /* new cell */
        cell_list->next =
            (struct cell_list_t *) malloc(sizeof(struct cell_list_t));
        assert(cell_list->next);
        if (!cell_list->next) {
            perror("add_cell_to_list");
            exit(1);
        }
        pred = cell_list;
        cell_list = cell_list->next;
    }

    /* init cell */
    cell_list->cell = cell;
    cell_list->next = NULL;
    cell_list->pred = pred;
    /*nb_cell++; */

    return (ini);
}

/** \brief return the number of items in the cell list
 **/
int cell_list_get_nb_item(struct cell_list_t *l)
{
    int nb = 0;

    if (!l)
        return (0);
    while (l) {
        nb++;
        l = l->next;
    }

    return (nb);
}

/** \brief  remove a cell from a given list
 **/
struct cell_list_t *remove_cell_from_list(struct cell_list_t *cell_list,
                                          struct cell_t *cell)
{

    struct cell_list_t *ini = cell_list;

    while (cell_list) {
        if (cell_list->cell == cell) {

            /*  update link infos */
            if (cell_list->pred == NULL && cell_list->next == NULL) {   /* first & last */
                free(cell_list);
                return (NULL);
            }

            if (cell_list->pred == NULL) {      /* first item */
                cell_list->next->pred = NULL;
                ini = cell_list->next;
            } else if (cell_list->next == NULL) {       /* last item  */
                cell_list->pred->next = NULL;
            } else {            /*  item n */
                cell_list->pred->next = cell_list->next;
                cell_list->next->pred = cell_list->pred;
            }

            /* delete item */
            free(cell_list);
            return (ini);
        }

        cell_list = cell_list->next;
    }
    return (ini);
}

/**  \brief destroy a cell_list 
 *
 *    It destroys a cell_list but not the cell.
 **/
void destroy_cell_list(struct cell_list_t *cell_list)
{
    struct cell_list_t *cell_list_to_free;

    while (cell_list != NULL) {
        cell_list_to_free = cell_list;
        cell_list = cell_list->next;
        free(cell_list_to_free);
    }
}

/**  \brief  add a neighbour to a given cell.
 **/
struct cell_list_t *add_neighbour_to_cell(struct cell_t *c1,
                                          struct cell_t *c2, int dir)
{
    c1->neighbour_list[dir] =
        add_cell_to_list(c1->neighbour_list[dir], c2);

    return (c1->neighbour_list[dir]);
}

/** \brief shows all the cell information
 *
 *   It shows all the cell information, such as  neighborhood, score, 
 *   points coordinates (if *tp is not NULL).
 **/
void dump_cell(struct point3d_tab_t *tp, struct cell_t *c, char *s)
{
    int i;

    /*struct point3d_t *tab; */

    fprintf(stderr, "#[x=%d,y=%d,z=%d] %s %p - score ", c->id.x, c->id.y,
            c->id.z, s, c);

    /* show score */
    if (c->cell_info == NULL) {
        for (i = 0; i < NB_SCORE; i++) {
            fprintf(stderr, "%f ", 0.0);
        }
    } else {
        for (i = 0; i < NB_SCORE; i++) {
            fprintf(stderr, "%f ", c->cell_info->score[i]);
        }
    }

    fprintf(stderr, "\n");
    if (c->point[0] == -1)
        fprintf(stderr, "no metric found\n");
    else {
        fprintf(stderr, "Points : ");
        for (i = 0; i < 8; i++)
            fprintf(stderr, "%d ", c->point[i]);

        fprintf(stderr, "\n");

        /*tab = *tp->tab + c->point[0];
           fprintf (stderr, "%3.1f:%3.1f:%3.1f\n", 
           tab->lat,
           tab->lon,
           tab->prf); */
    }

    /* neighbour dump */
    for (i = 0; i < 6; i++) {
        fprintf(stderr, "[%5s]: ", DIRECTION[i]);
        dump_cell_list(c->neighbour_list[i]);
    }
    fprintf(stderr, "\n");
}

/** \brief shows the cells elements in the cell list */
void dump_cell_list(struct cell_list_t *cl)
{
    while (cl) {
        fprintf(stderr, "maille(%p)[x=%d,y=%d,z=%d] ", cl->cell,
                cl->cell->id.x, cl->cell->id.y, cl->cell->id.z);
        cl = cl->next;
    }
    fprintf(stderr, "\n");
}

/** \brief given a rank in a cell list, it returns the pointer to the cell
 **/
struct cell_t *get_cell_from_list(struct cell_list_t *cl, int indice)
{
    int i = 0;

    if (indice < 0 || !cl)
        return (NULL);

    while (cl) {
        if (indice == i)
            return (cl->cell);
        cl = cl->next;
        i++;
    }

    return (NULL);
}

/** \brief feed a cell array with one more cell 
 **/
struct cell_tab_t *add_cell_to_tab(struct cell_tab_t *tabcell,
                                   struct cell_t *cell)
{
    if (tabcell->nbcell % tabcell->blockminsize == 0) {
        /* allocate new block */
        tabcell->tab = (struct cell_t **) realloc(tabcell->tab,
                                                  (tabcell->nbcell +
                                                   tabcell->blockminsize) *
                                                  sizeof(struct cell_t *));
        assert(tabcell->tab);
    }
    tabcell->tab[tabcell->nbcell] = cell;
    tabcell->nbcell++;

    return (tabcell);
}

/**  \brief  connect cells  together and share points 
 **/
void connect_cell(struct cell_t *c1, struct cell_t *c2, int *pt1, int *pt2)
{
    int i;

    for (i = 0; i < 4; i++)
        c1->point[pt1[i]] = c2->point[pt2[i]];
}

/**  \brief given a direction return the opposite direction 
 **/
int opposite_dir(int dir)
{
    switch (dir) {
    case NSD_1:
        return (NSD_2);
    case NSD_2:
        return (NSD_1);
    case EAST_D:
        return (WEST_D);
    case WEST_D:
        return (EAST_D);
    case UP_D:
        return (DOWN_D);
    case DOWN_D:
        return (UP_D);
    default:
        fprintf(stderr, "opposite_dir : Internal error : dir = %d\n", dir);
        exit(1);
    }
    return (-1);
}

/*----------------------*/

/* set point2create tab */

/*----------------------*/
void set_point2create(int *pc, int *face, int val)
{
    int i;

    for (i = 0; i < 4; i++) {
        pc[face[i]] = val;
    }
}

/** \brief  if *all* points in cell's face are not set  return 1
 **/
int edge_cell_without_point(struct cell_t *cell, int *face)
{
    int i;

    int empty = 0;

    for (i = 0; i < 4; i++) {
        if (cell->point[face[i]] == -1)
            empty++;
    }
    if (empty == 4)
        return (1);
    return (0);
}

/** \brief  return 1 if all points in cell *must NOT* be created 
 **/
int all_points_set(int *p2c)
{
    int i;

    for (i = 0; i < 8; i++)
        if (p2c[i] == 1)
            return (0);
    return (1);
}

void dump_point2create(int *pc)
{
    int i;

    for (i = 0; i < 8; i++) {
        fprintf(stderr, "[%d]=%d, ", i, pc[i]);
    }
    fprintf(stderr, "\n");
}

struct cell_t *cell_move(struct cell_t *c, int sens, int pas)
{
    int n = 0;

    while (n < pas && c->neighbour_list[sens] != NULL) {
        c = get_cell_from_list(c->neighbour_list[sens], 0), n++;
    }
    return (c);
}

/** \brief update cell's points coordinates given it's cells neighborhood                 
 **/
void
updateinfo_cell(struct cell_t *cell,
                float lat, float lon, float prf,
                float plat, float plon, float pprf,
                struct point3d_tab_t *point3d_tab)
{

    int i;

    int dir;

    int point2create[8];

    struct cell_t *voisin;

    int tab_face[6][4];
    int SensList[6] = { WEST_D, EAST_D, NSD_1, NSD_2, UP_D, DOWN_D };
    /*int SensList[6] = { NSD_1, NSD_2, EAST_D, WEST_D, UP_D, DOWN_D }; */
    int face_3267[4] = { 3, 2, 6, 7 };
    int face_0154[4] = { 0, 1, 5, 4 };

    tab_face[EAST_D][0] = 1;
    tab_face[EAST_D][1] = 2;
    tab_face[EAST_D][2] = 6;
    tab_face[EAST_D][3] = 5;
    tab_face[WEST_D][0] = 0;
    tab_face[WEST_D][1] = 3;
    tab_face[WEST_D][2] = 7;
    tab_face[WEST_D][3] = 4;
    tab_face[DOWN_D][0] = 0;
    tab_face[DOWN_D][1] = 1;
    tab_face[DOWN_D][2] = 2;
    tab_face[DOWN_D][3] = 3;
    tab_face[UP_D][0] = 4;
    tab_face[UP_D][1] = 5;
    tab_face[UP_D][2] = 6;
    tab_face[UP_D][3] = 7;

    if (modulo360(lon, 0) > 0. - EPS && modulo360(lon, 0) < 180. - EPS) {
        if (DEBUG)
            fprintf(stderr,
                    "updateinfo_cell : cell is in [0,180[ (%.1f)\n",
                    modulo360(lon, 0));
        tab_face[NSD_1][0] = 3;
        tab_face[NSD_1][1] = 2;
        tab_face[NSD_1][2] = 6;
        tab_face[NSD_1][3] = 7;

        tab_face[NSD_2][0] = 0;
        tab_face[NSD_2][1] = 1;
        tab_face[NSD_2][2] = 5;
        tab_face[NSD_2][3] = 4;
    } else {
        if (DEBUG)
            fprintf(stderr,
                    "updateinfo_cell : cell is in [180,360[ (%.1f)\n",
                    modulo360(lon, 0));
        tab_face[NSD_1][0] = 0;
        tab_face[NSD_1][1] = 1;
        tab_face[NSD_1][2] = 5;
        tab_face[NSD_1][3] = 4;

        tab_face[NSD_2][0] = 3;
        tab_face[NSD_2][1] = 2;
        tab_face[NSD_2][2] = 6;
        tab_face[NSD_2][3] = 7;
    }

    /* all points must be created */
    for (i = 0; i < 8; i++)
        point2create[i] = 1;

    /* for each cell face */
    for (i = 0; i < 6; i++) {
        dir = SensList[i];

        if (cell->neighbour_list[dir] == NULL)
            continue;

        voisin = get_cell_from_list(cell->neighbour_list[dir], 0);

        if (all_points_set(point2create))
            break;

        if (lat > 90. - plat - EPS && (dir == NSD_1 || dir == NSD_2)) {

            /* north pole + dir = NSD_1 |  NSD_2 */
            switch (dir) {
            case NSD_1:
                if (modulo360(lon, 0) > 0. - EPS
                    && modulo360(lon, 0) < 180. - EPS) {
                    /* north pole [0,180[ */
                    if (edge_cell_without_point(voisin, face_3267))
                        continue;
                    connect_cell(cell, voisin, face_3267, face_3267);
                    set_point2create(point2create, face_3267, 0);

                } else {
                    /* north pole [180,360[ */
                    if (edge_cell_without_point(voisin, face_3267))
                        continue;
                    connect_cell(cell, voisin, face_0154, face_3267);
                    set_point2create(point2create, face_0154, 0);
                }
                break;
            case NSD_2:
                if (modulo360(lon, 0) > 0. - EPS
                    && modulo360(lon, 0) < 180. - EPS) {
                    /* north pole [0,180[ */
                    if (edge_cell_without_point(voisin, face_3267))
                        continue;
                    connect_cell(cell, voisin, face_0154, face_3267);
                    set_point2create(point2create, face_0154, 0);

                } else {
                    /* north pole [180,360[ */
                    if (edge_cell_without_point(voisin, face_3267))
                        continue;
                    connect_cell(cell, voisin, face_3267, face_3267);
                    set_point2create(point2create, face_3267, 0);
                }
                break;
            }
        } else if (lat < -90. + EPS && (dir == NSD_1 || dir == NSD_2)) {
            /* south pole + dir = NSD_1 |  NSD_2 */
            switch (dir) {
            case NSD_1:
                if (modulo360(lon, 0) > 0. - EPS
                    && modulo360(lon, 0) < 180. - EPS) {
                    /* south pole [0,180[ */
                    if (edge_cell_without_point(voisin, face_0154))
                        continue;
                    connect_cell(cell, voisin, face_3267, face_0154);
                    set_point2create(point2create, face_3267, 0);

                } else {
                    /* south pole [180,360[ */
                    if (edge_cell_without_point(voisin, face_0154))
                        continue;
                    connect_cell(cell, voisin, face_0154, face_0154);
                    set_point2create(point2create, face_0154, 0);
                }
                break;
            case NSD_2:
                if (modulo360(lon, 0) > 0. - EPS
                    && modulo360(lon, 0) < 180. - EPS) {
                    if (edge_cell_without_point(voisin, face_0154))
                        continue;
                    connect_cell(cell, voisin, face_0154, face_0154);
                    set_point2create(point2create, face_0154, 0);

                } else {
                    if (edge_cell_without_point(voisin, face_0154))
                        continue;
                    connect_cell(cell, voisin, face_3267, face_0154);
                    set_point2create(point2create, face_3267, 0);
                }
                break;
            }
        } else {
            /* could be a pole with dir = EAST_D|WEST_D */
            /* lat = ]-90,90[  */
            if (edge_cell_without_point
                (voisin, tab_face[opposite_dir(dir)]))
                continue;
            connect_cell(cell, voisin, tab_face[dir],
                         tab_face[opposite_dir(dir)]);
            set_point2create(point2create, tab_face[dir], 0);
        }
    }

    if (DEBUG) {
        create_points_for_cell(cell, lat, lon, prf, plat, plon, pprf,
                               point2create, point3d_tab,
                               "updateinfo_cell :");
    } else {
        create_points_for_cell(cell, lat, lon, prf, plat, plon, pprf,
                               point2create, point3d_tab, NULL);
    }

}

/**
 * \brief assign geographic coordinates to cell vertices
 * 
 *  given the geographic coordinates (lat,lon,depth) of 
 *  the point "0" of the cell, and the dimensions of the cell in all 3 directions
 *  (plat,plon,pdepth), computes and assign geographic coordinates to the 8 cell 
 *  vertices stored in the points structure member. 
 *  Additional input parameters are : 
 *
 *    @param cell an allocated cell without associated point
 *    @param lat
 *    @param lon
 *    @param depth
 *    @param plat
 *    @param plon
 *    @param pdepth
 *    @param point2create an 8 int array that indicate if the corresponding vertices should be created or not.                                                
 *    @param point3d_tab structure where to store the allocated points
 *    @param texte if non null, outputs some informations on vertices created.
 *
 * Vertices numbering :
 * \verbatim                            lat
                                        ^   depth
                                        | / 
                                        |/
                                        +---> lon
        7 +---------+ 6                 
         /|        /|                   
        / |       / |                  x
     3 +--+------+ 2|                  ^   z
       |  |      |  |                  | /    
       |4 +------+--+ 5                |/
       | /       | /                   +---> y
       |/        |/                   
     0 +---------+ 1                 

   \endverbatim
 **/
void
create_points_for_cell(struct cell_t *cell,
                       float lat, float lon, float depth,
                       float plat, float plon, float pdepth,
                       int *point2create,
                       struct point3d_tab_t *point3d_tab, char *texte)
{
    int i;

    int nbcree = 0;

    int freeok = 0;

    if (point2create == NULL) {
        /* all points must be created */
        point2create = (int *) malloc(8 * sizeof(int));
        assert(point2create);
        for (i = 0; i < 8; i++)
            point2create[i] = 1;
        freeok = 1;
    }

    for (i = 0; i < 8; i++) {
        if (point2create[i] == 0)
            continue;
        nbcree++;
        switch (i) {
        case 0:
            cell->point[i] = create_point3d(point3d_tab, lat, lon, depth);
            break;
        case 1:
            cell->point[i] =
                create_point3d(point3d_tab, lat, lon + plon, depth);
            break;
        case 2:
            cell->point[i] =
                create_point3d(point3d_tab, lat + plat, lon + plon, depth);
            break;
        case 3:
            cell->point[i] =
                create_point3d(point3d_tab, lat + plat, lon, depth);
            break;
        case 4:
            cell->point[i] =
                create_point3d(point3d_tab, lat, lon, depth + pdepth);
            break;
        case 5:
            cell->point[i] =
                create_point3d(point3d_tab, lat, lon + plon,
                               depth + pdepth);
            break;
        case 6:
            cell->point[i] =
                create_point3d(point3d_tab, lat + plat, lon + plon,
                               depth + pdepth);
            break;
        case 7:
            cell->point[i] =
                create_point3d(point3d_tab, lat + plat, lon,
                               depth + pdepth);
            break;
        }
    }

    if (texte != NULL) {
        fprintf(stderr, "%s nb points crees = %d [ ", texte, nbcree);
        for (i = 0; i < 8; i++) {
            if (point2create[i])
                fprintf(stderr, "%d ", i);
        }
        fprintf(stderr, "]\n");
    }

    if (freeok)
        free(point2create);
}

/**
 * \brief return the geographic coordinates of the 4 vertices of a cell face.
 * 
 *  given the cell pointer, and a face number, return a  
 *  pointer to a 4 coord_geo_t elements array. 
 **/

struct point3d_t *get_face_vertices(struct cell_t *cell, int face,
                                    struct point3d_tab_t *point3d_tab)
{
    int i;

    int faces_indices[6][4] = {

        {3, 2, 6, 7},/**< NSD_1  */

        {0, 1, 5, 4},/**< NSD_2  */

        {1, 2, 6, 5},/**< EAST_D */

        {0, 3, 7, 4},/**< WEST_D */

        {0, 1, 2, 3},/**< UP_D */

        {4, 5, 6, 7} /**< DOWN_D  */
        /* array order matters */
    };

    struct point3d_t *vertices = calloc(4, sizeof(struct point3d_t));

    for (i = 0; i < 4; i++) {
        vertices[i] =
            *(point3d_tab->tab[cell->point[faces_indices[face][i]]]);
    }
    return (vertices);
}
