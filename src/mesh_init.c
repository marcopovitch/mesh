#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>             /* stat */
#include <sys/mman.h>           /* stat */
#include <libgen.h>             /* dirname, basename */

#include <math.h>

#include <string.h>
#include <popt.h>

#include <libxml/tree.h>        /* xml */
#include <libxml/parser.h>      /* xml */

#include "const.h"
#include "point3d.h"
#include "cell.h"
#include "mesh.h"
#include "layer.h"
#include "modulo.h"
#include "export_vtk.h"
#include "import.h"
#include "extern.h"

#if defined(SOLARIS)
#define drem remainder
#endif

/* these variables are declared extern in other source files */

int VERBOSE = 0;                /**< show importants informations */

int DEBUG = 0;                  /**< debuging stuff */

int ALLOC_ALL_POINTS = 0;               /**< if FALSE 2 adacents cells share 4 points

					   (4 alloc rather than 8)
					   usefull to see all the edges */

int DO_NOT_LINK_LAYERS = 0;             /**< if TRUE don't link the layer together

					   usefull when using VTK viewer */

/* score info */
int NB_SCORE;

char **SCORE_NAME;

/* file included in xml */
const int NB_MESH_FILE_FORMAT = 6;

const char *MESH_FILE_FORMAT[] = {
    "r2m",                      /* file produced by ray2mesh */
    "sco",                      /* file produced by ray2mesh */
    "res",                      /* file produced by ray2mesh */
    "sparse",                   /* file produced by ray2mesh */
    "evt",                      /* file produced by ray2mesh */
    "irm"                       /* file produced by gimbos */
};

const char *DIRECTION[] =
    { "NSD_1", "NSD_2", "EAST", "WEST", "UP", "DOWN" };

/* end these variables are declared extern in other source files */

/*
 * default mesh parameters : latmin, latmax, lonmin, lonmax, lat_unit_size,
 * lon_unit_size
 */
struct mesh_parameter_t param_base = { -90, 90, 0, 360, 10, 10 };

/* recognized mesh attributes (keep it coherent with update_param()) */
const char *MESH_TAG = "mesh";

const int MESH_NPROPS = 6;

const char *MESH_PROPS[] = {
    "lat-min", "lat-max",
    "lon-min", "lon-max",
    "lat-unit-size", "lon-unit-size"
};

/* recognized layer attributes (keep it coherent with update_param()) */
const char *MODEL_TAG = "model";

const char *LAYER_TAG = "layer";

const int LAYER_NPROPS = 6;

const char *LAYER_PROPS[] = {
    "number", "name",
    "zstart", "zend",
    "lat-unit", "lon-unit"
};

/* recognized data attributes (keep it coherent with update_param()) */
const char *DATA_TAG = "data";

const int DATA_NPROPS = 2;

const char *DATA_PROPS[] = {
    "format", "directory"
};

const char *FILE_TAG = "file";

const int FILE_NPROPS = 1;
const char *FILE_PROPS[] = { "name" };

int get_file_format_in_xml(char *format)
{
    int i;

    for (i = 0; i < NB_MESH_FILE_FORMAT; i++) {
        if (!strcmp(MESH_FILE_FORMAT[i], format)) {
            return (i);
        }
    }
    return (-1);
}

/**\brief overwrites standard mesh parameters.
 *
 * update_mesh_parameter : overwrites standard mesh parameters.
 * Each time a new xml tag is encountered when parsing the conf file
 * this function is called and passed the tagnum associated with the tag,
 * and the attribute value s.
 * @param mesh the mesh address (to access mes->parameter)
 * @param tagnum a tag number (int) in [0..LAYER_NPROPS-1]
 * @param s the tag value (the string is parsed as a float)
 */
void update_mesh_parameter(struct mesh_t *mesh, int tagnum, char *s)
{

    switch (tagnum) {
    case 0:
        mesh->parameter->lat_min = atof(s);
        break;
    case 1:
        mesh->parameter->lat_max = atof(s);
        break;
    case 2:
        mesh->parameter->lon_min = atof(s);
        break;
    case 3:
        mesh->parameter->lon_max = atof(s);
        break;
    case 4:
        mesh->parameter->lat_unit_size = atof(s);
        break;
    case 5:
        mesh->parameter->lon_unit_size = atof(s);
        break;
    }
}

/*-------------------------------------------------------------------------*/

/* update_layer_param : overrides standards layer parameters               */

/*-------------------------------------------------------------------------*/
void update_layer_parameter(struct layer_t *layer, int tagnum, char *s)
{

    switch (tagnum) {
    case 0:
        layer->number = atof(s);
        break;
    case 1:
        free(layer->name);
        layer->name = strdup(s);
        assert(layer->name);
        break;
    case 2:
        layer->zstart = atof(s);
        break;
    case 3:
        layer->zend = atof(s);
        break;
    case 4:
        layer->lat_unit = atof(s);
        break;
    case 5:
        layer->lon_unit = atof(s);
        break;
    }
}

/*-------------------------------------------------------------------------*/

/* layer_init: creates layers                                              */

/*-------------------------------------------------------------------------*/
void layer_init(struct mesh_t *mesh)
{
    int l;

    mesh->layer =
        (struct layer_t **) malloc(mesh->nlayers *
                                   sizeof(struct layer_t *));
    assert(mesh->layer);

    for (l = 0; l < mesh->nlayers; l++) {
        mesh->layer[l] = (struct layer_t *) malloc(sizeof(struct layer_t));
        assert(mesh->layer[l]);
        mesh->layer[l]->name = strdup("noname");
        assert(mesh->layer[l]->name);
        mesh->layer[l]->number = l;
        mesh->layer[l]->ncells = 0;
        mesh->layer[l]->nlon = 0;
        mesh->layer[l]->nlat = 0;
        mesh->layer[l]->npoints = 0;
        mesh->layer[l]->cell = NULL;
        mesh->layer[l]->zstart = 0.;
        mesh->layer[l]->zend = 0.;
        mesh->layer[l]->lat_unit = 1;
        mesh->layer[l]->lon_unit = 1;
    }
}

void update_layer_info(struct mesh_t *mesh)
{
    int l;

    struct mesh_parameter_t *mp;

    mp = mesh->parameter;
    mesh->ncells = 0;

    for (l = 0; l < mesh->nlayers; l++) {
        /* nlat */
        if (!mesh->layer[l]) {
            continue;
        }
        mesh->layer[l]->nlat = (int) rint(((mp->lat_max - mp->lat_min) /
                                           (mesh->layer[l]->lat_unit *
                                            mp->lat_unit_size)));

        /* nlon */
        mesh->layer[l]->nlon = (int) rint(((mp->lon_max - mp->lon_min) /
                                           (mesh->layer[l]->lon_unit *
                                            mp->lon_unit_size)));

        mesh->layer[l]->ncells =
            mesh->layer[l]->nlat * mesh->layer[l]->nlon;
        mesh->ncells += mesh->layer[l]->ncells;
    }
}

/*-------------------------------------------------------------------------*/

/* check_overlapping : exit if layers depths are incoherent                */

/*-------------------------------------------------------------------------*/
void check_overlapping(struct mesh_t *mesh)
{
    int l;

    for (l = 1; l < mesh->nlayers; l++) {
        if (mesh->layer[l]->zstart < mesh->layer[l - 1]->zend) {
            fprintf(stderr,
                    "mesh:check_overlapping: incorrect parameters : layers overlapping \n");
            fprintf(stderr,
                    "mesh:check_overlapping: end of layer %d is at %.2f kms\n",
                    l - 1, mesh->layer[l - 1]->zend);
            fprintf(stderr,
                    "mesh:check_overlapping: start of layer %d is at %.2f kms\n",
                    l, mesh->layer[l]->zstart);
            exit(1);
        }
    }
}

/** \brief cells must pad *ALL* the mesh
  * and a cell in a given layer must have 'n' whole
  * neighbour cells (n is an integer)
  */
void check_cell_size(struct mesh_t *mesh)
{
    int l;

    double tmp;

    for (l = 0; l < mesh->nlayers; l++) {

        if ((tmp =
             fabs(drem(mesh->parameter->lat_max - mesh->parameter->lat_min,
                       mesh->layer[l]->lat_unit *
                       mesh->parameter->lat_unit_size))) > 0.001) {
            fprintf(stderr,
                    "check_cell_size : incorrect parameters in layer #%d (\"%s\") : choose another lat-unit [%f]\n",
                    l, mesh->layer[l]->name, tmp);
            fprintf(stderr,
                    "check_cell_size : an integer number of cells must pad your domain !\n");
            exit(1);
        }
        if ((tmp =
             fabs(drem(mesh->parameter->lon_max - mesh->parameter->lon_min,
                       mesh->layer[l]->lon_unit *
                       mesh->parameter->lon_unit_size))) > 0.001) {
            fprintf(stderr,
                    "check_cell_size : incorrect parameters in layer #%d (\"%s\"): choose another lon-unit [%f]\n",
                    l, mesh->layer[l]->name, tmp);
            fprintf(stderr,
                    "check_cell_size : an integer number of cells must pad your domain !\n");
            exit(1);
        }
        if (l == 0)
            continue;

        if ((mesh->layer[l]->lat_unit % mesh->layer[l - 1]->lat_unit) &&
            (mesh->layer[l - 1]->lat_unit % mesh->layer[l]->lat_unit)) {
            fprintf(stderr,
                    "check_cell_size : incorrect parameters in layer #%d or #%d: choose an other lat-unit\n",
                    l - 1, l);
            fprintf(stderr,
                    "check_cell_size : lat-unit should be n*lat_unit of the last layer (or vice & versa)!\n");
            exit(1);
        }
        if ((mesh->layer[l]->lon_unit % mesh->layer[l - 1]->lon_unit) &&
            (mesh->layer[l - 1]->lon_unit % mesh->layer[l]->lon_unit)) {
            fprintf(stderr,
                    "check_cell_size : incorrect parameters in layer #%d or #%d: choose an other lon-unit\n",
                    l - 1, l);
            fprintf(stderr,
                    "check_cell_size : lon-unit should be n*lon_unit of the last layer (or vice & versa)!\n");
            exit(1);
        }
    }
}

/*-----------------------------------------------------------------------------------------*
 *\brief check_mesh_parameter  : check mesh parameters consistancy
 *-----------------------------------------------------------------------------------------*/
void check_mesh_parameter(struct mesh_t *mesh)
{
    struct mesh_parameter_t *mp;

    float diff_lon, diff_lat;

    int l;

    int nb_max_meridian;

    mp = mesh->parameter;
    diff_lon = mp->lon_max - mp->lon_min;
    diff_lat = mp->lat_max - mp->lat_min;

    if (diff_lon > 360. || diff_lon < EPS) {
        fprintf(stderr,
                "check_mesh_parameter : incorrect parameters for longitude mesh bounds\n");
        exit(1);
    }
    if (diff_lat > 180. || diff_lat < EPS) {
        fprintf(stderr,
                "check_mesh_parameter : incorrect parameters for latitude mesh bounds\n");
        exit(1);
    }
    mp->lon_min = modulo(mp->lon_min, 360);
    if (mp->lon_min < 0)
        mp->lon_min += 360.;
    mp->lon_max = mp->lon_min + diff_lon;

    if (DEBUG) {
        fprintf(stderr,
                "check_mesh_parameter : lon_min = %.2f, lonmax= %.2f\n",
                mp->lon_min, mp->lon_max);
    }
    /* GLOBAL */
    /* FIXME :  check the POLES !! */
    if (mp->lon_max - mp->lon_min < 360.0) {
        return;
    }
    /*
     * FIXEME : nb_max_meridian must be odd number in a GLOBAL mesh
     * (links between meridians)
     */
    for (l = 0; l < mesh->nlayers; l++) {
        nb_max_meridian =
            (int) (360. / (mesh->layer[l]->lon_unit * mp->lon_unit_size));
        if (nb_max_meridian % 2) {
            fprintf(stderr,
                    "check_mesh_parameter : nb max meridian must be odd number (see layer[%d])\n",
                    l);
            exit(1);
        }
    }
}

void
select_mesh_layer(struct mesh_t *mesh, int *begin_layer, int *end_layer)
{
    int layer;

    if (*begin_layer == -1)
        *begin_layer = 0;
    if (*end_layer == -1)
        *end_layer = mesh->nlayers - 1;

    if ((*end_layer >= mesh->nlayers) ||
        (*end_layer < *begin_layer) || (*begin_layer < 0)) {
        printf("Incorrect layers specification (got begin=%d end=%d)\n",
               *begin_layer, *end_layer);
        printf("Check that - end-layer > begin-layer\n");
        printf("           - end-layer is between 0 and %d\n",
               mesh->nlayers - 1);
        exit(1);
    }
    /* clean the structure for layers we don't want to process */
    for (layer = 0; layer < mesh->nlayers; layer++) {
        if (layer < *begin_layer || layer > *end_layer) {
            free(mesh->layer[layer]->name);
            free(mesh->layer[layer]);
            mesh->layer[layer] = NULL;
        }
    }
}

/*-------------------------------------------------------------------------
 *\brief :  is_valid_attribute: check that attribname belongs to validnames
 *-------------------------------------------------------------------------*/
static int
is_valid_attribute(char *attribname, const char *validnames[], int nbnames)
{
    int i;

    int tag_found = 0;

    for (i = 0; i < nbnames && !tag_found; i++) {
        if (!strcmp(attribname, validnames[i]))
            tag_found = 1;
    }
    return (tag_found);
}

int xml_count_leaves(xmlNodePtr node, const char *tag)
{
    xmlNodePtr curr;

    int nb = 0;

    curr = node->xmlChildrenNode;
    while (curr) {
        if (!xmlStrcmp(curr->name, (const xmlChar *) tag)) {
            nb++;
        }
        curr = curr->next;
    }
    return (nb);
}

int
xml_check_attributes(xmlNodePtr node, const char *props[],
                     const int nprops)
{
    xmlAttrPtr attr;

    for (attr = node->properties; attr != NULL; attr = attr->next) {
        if (!is_valid_attribute((char *) attr->name, props, nprops)) {
            fprintf(stderr,
                    "xml_check_attributes() : unkown attribute \"%s\" for tag <%s>\n",
                    attr->name, node->name);
            return (0);
        }
        if (DEBUG) {
            fprintf(stderr, "xml_check_attributes() : (%s)='%s'\n",
                    (char *) attr->name, xmlGetNsProp(node,
                                                      (char *) attr->name,
                                                      NULL));
        }
    }
    return (1);
}

/**\brief creates the mesh and mesh_parameter structure  */

static struct mesh_t *mesh_alloc(struct mesh_parameter_t *param_base)
{
    struct mesh_t *mesh;

    /* create the mesh and mesh_parameter and mesh_data */
    mesh = (struct mesh_t *) malloc(sizeof(struct mesh_t));
    assert(mesh);

    /* point allocator */
    mesh->allocated_points =
        (struct point3d_tab_t *) malloc(sizeof(struct point3d_tab_t));
    assert(mesh->allocated_points);
    mesh->allocated_points->tab = NULL;
    mesh->allocated_points->nbpoint = 0;
    mesh->allocated_points->blockminsize = 1000;

    /* cell allocator */
    mesh->allocated_cells =
        (struct cell_tab_t *) malloc(sizeof(struct cell_tab_t));
    assert(mesh->allocated_cells);
    mesh->allocated_cells->tab = NULL;
    mesh->allocated_cells->nbcell = 0;
    mesh->allocated_cells->blockminsize = 1000;

    mesh->overlap = NULL;

    mesh->parameter = (struct mesh_parameter_t *)
        malloc(sizeof(struct mesh_parameter_t));
    assert(mesh->parameter);
    memcpy(mesh->parameter, param_base, sizeof(struct mesh_parameter_t));

    mesh->data = (struct mesh_data_t **)
        calloc(NB_MESH_FILE_FORMAT, sizeof(struct mesh_data_t *));
    assert(mesh->data);

    mesh->ncells = 0;
    mesh->nlayers = 0;
    mesh->layer = NULL;
    mesh->cell = NULL;
    mesh->xml_filename = NULL;
    mesh->max_rayid = -1;

    return (mesh);
}

struct mesh_overlap_t *mesh_overlap_alloc(struct mesh_t *mesh)
{
    struct mesh_overlap_t *mo;

    mo = (struct mesh_overlap_t *) malloc(sizeof(struct mesh_overlap_t));
    assert(mo);
    mesh->overlap = mo;

    mo->base_parameter = (struct mesh_parameter_t *)
        malloc(sizeof(struct mesh_parameter_t));
    assert(mo->base_parameter);

    mo->first_cell_in_noz =
        (struct coord_z3_t *) malloc(sizeof(struct coord_z3_t));
    assert(mo->first_cell_in_noz);

    mo->last_cell_in_noz =
        (struct coord_z3_t *) malloc(sizeof(struct coord_z3_t));
    assert(mo->last_cell_in_noz);

    mo->first_cell_in_z1 =
        (struct coord_z3_t *) malloc(sizeof(struct coord_z3_t));
    assert(mo->first_cell_in_z1);

    mo->last_cell_in_z1 =
        (struct coord_z3_t *) malloc(sizeof(struct coord_z3_t));
    assert(mo->last_cell_in_z1);

    return (mo);
}

/**\brief Parse an XML document and return a mesh whose parameters have been set up.
 *
 * Takes an xml AST, and a string that can be considered a comment.
 * The comment is aimed at storing the filename of the xml description.
 * The xml AST is used to fill in the mesh parameters. Once filled, parameters
 * consitenct is checked.

 * @param doc the XML AST as parsed by libxml2
 * @param string
 * global : reads param_base
 **/
static struct mesh_t *mesh_init_parsing(xmlDocPtr doc, char *filename)
{
    struct mesh_t *mesh;

    xmlNodePtr node, curr, child, layer_node = NULL;

    xmlAttrPtr attr;

    char *buffer;

    int nb_layers, nb_section, nb_file;

    int i, l;

    int format = -1;

    mesh = mesh_alloc(&param_base);
    if (filename) {
        mesh->xml_filename = strdup(filename);
        assert(mesh->xml_filename);
    }
    /* first check that mesh attributes belongs to known keywords */
    node = xmlDocGetRootElement(doc);

    if (!xml_check_attributes(node, MESH_PROPS, MESH_NPROPS)) {
        free(mesh->parameter);
        free(mesh);
        return (NULL);
    }
    /* parse MESH attributes */
    for (attr = node->properties; attr != NULL; attr = attr->next) {
        for (i = 0; i < MESH_NPROPS; i++) {
            buffer = NULL;
            buffer = xmlGetNsProp(node, (xmlChar *) MESH_PROPS[i], NULL);
            if (buffer) {
                /* alter the param_base defaults */
                update_mesh_parameter(mesh, i, buffer);
                free(buffer);
            }
        }
    }

    /* count items in LAYER and DATA node */
    nb_layers = 0;
    nb_section = 0;
    curr = node->xmlChildrenNode;
    while (curr) {
        if (!xmlStrcmp(curr->name, (const xmlChar *) MODEL_TAG)) {
            /* how many layer ? */
            layer_node = curr;
            nb_layers = xml_count_leaves(curr, LAYER_TAG);

        } else {

            if (!xmlStrcmp(curr->name, (const xmlChar *) DATA_TAG)) {
                if (!xml_check_attributes(curr, DATA_PROPS, DATA_NPROPS)) {
                    free(mesh->parameter);
                    free(mesh);
                    return (NULL);
                }
            } else {
                /* unknow entry : skip it */
                curr = curr->next;
                continue;
            }

            nb_section++;

            /* data section */
            for (i = 0; i < DATA_NPROPS; i++) {
                buffer = NULL;
                buffer = xmlGetProp(curr, (xmlChar *) DATA_PROPS[i]);
                if (!buffer) {
                    continue;
                }
                /* format */
                if (!strcmp(DATA_PROPS[i], "format")) {
                    format = get_file_format_in_xml(buffer);
                    if (format < 0) {
                        fprintf(stderr,
                                "mesh_init_parsing: unknown format '%s'\n",
                                buffer);
                        free(mesh->parameter);
                        free(mesh);
                        return (NULL);
                    }
                    if (mesh->data[format]) {
                        fprintf(stderr,
                                "mesh_init_parsing: section with format '%s' already exists\n",
                                buffer);
                        free(mesh->parameter);
                        free(mesh);
                        return (NULL);
                    }
                    mesh->data[format] = (struct mesh_data_t *)
                        calloc(1, sizeof(struct mesh_data_t));
                    assert(mesh->data[format]);
                    mesh->data[format]->format = strdup(buffer);
                    assert(mesh->data[format]->format);

                    free(buffer);
                    continue;
                }
                /* directory */
                if (!strcmp(DATA_PROPS[i], "directory")) {
                    if (format < 0 || !mesh->data[format]) {
                        fprintf(stderr,
                                "mesh_init_parsing: format not yet defined !\n");
                        free(mesh->parameter);
                        free(mesh);
                        return (NULL);
                    }
                    mesh->data[format]->directory = strdup(buffer);
                    assert(mesh->data[format]->directory);
                    free(buffer);
                    continue;
                }
            }                   /* end for */

            /* Check files in the current <data> block */
            nb_file = 0;
            child = curr->xmlChildrenNode;

            if (!child) {
                curr = curr->next;
                continue;
            }
            mesh->data[format]->ndatafile =
                xml_count_leaves(curr, FILE_TAG);
            mesh->data[format]->filename = (char **)
                calloc(mesh->data[format]->ndatafile, sizeof(char *));
            assert(mesh->data[format]->filename);

            while (child) {
                if (!xmlStrcmp(child->name, (const xmlChar *) FILE_TAG)) {
                    if (!xml_check_attributes
                        (child, FILE_PROPS, FILE_NPROPS)) {
                        free(mesh->layer);
                        free(mesh->parameter);
                        free(mesh);
                        return (NULL);
                    }
                    /*
                     * override standarts mesh params
                     * with those in config file
                     */
                    for (i = 0; i < FILE_NPROPS; i++) {
                        buffer =
                            xmlGetProp(child, (xmlChar *) FILE_PROPS[i]);
                        if (!buffer) {
                            continue;
                        }
                        if (!strcmp(FILE_PROPS[i], "name")) {
                            char *tmp;

                            tmp = (char *)
                                malloc(sizeof(char) *
                                       (strlen
                                        (mesh->data[format]->directory) +
                                        strlen("/") + strlen(buffer) + 1));
                            sprintf(tmp, "%s/%s",
                                    mesh->data[format]->directory, buffer);

                            mesh->data[format]->filename[nb_file] =
                                strdup(tmp);
                            assert(mesh->data[format]->filename[nb_file]);
                            free(buffer);
                            free(tmp);
                            continue;
                        }
                        if (buffer) {
                            free(buffer);
                        }
                    }
                    nb_file++;
                }
                child = child->next;
            }                   /* while child */
        }                       /* else */
        curr = curr->next;
    }                           /* end while (cur) */

    if (!nb_layers) {
        fprintf(stderr,
                "mesh: no layer found. Probably missing <%s> or <%s> tags.\n",
                MODEL_TAG, LAYER_TAG);
        exit(1);
    }
    if (DEBUG) {
        fprintf(stderr, "%s: mesh_init_parsing() : found %d layers\n",
                __FILE__, nb_layers);
        fprintf(stderr, "%s: mesh_init_parsing():  found %d section(s)\n",
                __FILE__, nb_section);
    }

        /*******************/
    /* initialize mesh */

        /*******************/
    mesh->nlayers = nb_layers;
    layer_init(mesh);

    /* init meta-cell list to null */
    mesh->nb_total_metacell = 0;
    mesh->nb_metacell = (int *) calloc(nb_layers, sizeof(int));
    mesh->metacell =
        (struct cell_t ***) calloc(nb_layers, sizeof(struct cell_t **));
    assert(mesh->metacell);

    /* now, for each LAYER (a mesh child), */
    /* parse all layers attributes values and assign it to mesh layers */
    l = 0;
    if (layer_node) {
        curr = layer_node->xmlChildrenNode;
        while (curr) {
            if (!xmlStrcmp(curr->name, (const xmlChar *) LAYER_TAG)) {
                if (!xml_check_attributes(curr, LAYER_PROPS, LAYER_NPROPS)) {
                    free(mesh->layer);
                    free(mesh->parameter);
                    free(mesh);
                    return (NULL);
                }
                /*
                 * override standarts mesh params with those
                 * in config file
                 */
                for (i = 0; i < LAYER_NPROPS; i++) {
                    buffer = NULL;
                    buffer = xmlGetProp(curr, (xmlChar *) LAYER_PROPS[i]);
                    if (buffer) {
                        if (DEBUG) {
                            fprintf(stderr,
                                    "mesh_init_parsing: updating layer #%d with property %s=%s ...\n",
                                    l, LAYER_PROPS[i], buffer);
                        }
                        update_layer_parameter(mesh->layer[l], i, buffer);
                        free(buffer);
                    }
                }
                l++;
            }
            curr = curr->next;
        }
    }
    /* update layer info */
    update_layer_info(mesh);

    /* test for overlapping layers */
    check_overlapping(mesh);

    /* test for consistant cell */
    check_cell_size(mesh);

    /* check mesh_parameter */
    check_mesh_parameter(mesh);

    /* score is only set when we import ray2mesh data */
    NB_SCORE = 0;
    SCORE_NAME = NULL;

    return (mesh);
}

/*-------------------------------------------------------------------------*/

/* mesh_init_from_file :                                                   */

/* Takes an xml mesh config file and returns the corresponding structure   */

/*-------------------------------------------------------------------------*/
struct mesh_t *mesh_init_from_file(char *conffile)
{
    xmlDocPtr doc;

    struct mesh_t *m;

    if (!conffile)
        return (NULL);

    doc = xmlParseFile(conffile);
    if (doc == NULL) {
        fprintf(stderr,
                "(%s):mesh_init_from_file: could not parse file %s\n",
                __FILE__, conffile);
        xmlFreeDoc(doc);
        return (NULL);
    }
    if (xmlDocGetRootElement(doc) == NULL) {
        fprintf(stderr, "(%s):mesh_init_from_file: %s is empty\n",
                __FILE__, conffile);
        xmlFreeDoc(doc);
        return (NULL);
    }
    if (xmlStrcmp
        (xmlDocGetRootElement(doc)->name, (const xmlChar *) "mesh"))
        fprintf(stderr,
                "(%s):mesh_init_from_file: %s document of the wrong type \n",
                __FILE__, xmlDocGetRootElement(doc)->name);

    m = mesh_init_parsing(doc, conffile);
    xmlFreeDoc(doc);
    return (m);
}

/*-------------------------------------------------------------------------
 * \brief : load_file_to_memory : map a disk file to memory
 *
 * Given a filename, load the file contents to memory and return a pointer
 * to the start memory address of the file, and the length size of the zone
 *-------------------------------------------------------------------------
 */
char *load_file_to_memory(char *filename, int *size)
{
    struct stat info;

    char *buffer;

    int fd;

    *size = 0;
    if ((fd = open(filename, O_RDWR)) == -1) {
        return (NULL);
    }
    stat(filename, &info);
    if ((buffer =
         (char *) mmap(NULL, info.st_size, PROT_READ | PROT_WRITE,
                       MAP_PRIVATE, fd, 0)) == MAP_FAILED) {
        fprintf(stderr,
                "%s : load_file_to_memory() : could not map %d bytes to memory\n",
                PACKAGE, (int) info.st_size);
        perror("mmap error");
        return (NULL);
    }
    close(fd);
    *size = info.st_size;
    return (buffer);
}

/*-------------------------------------------------------------------------*/

/* unload_file_from_memory :                                               */

/* Given an address, unallocates size bytes from address                   */

/*-------------------------------------------------------------------------*/
void unload_file_from_memory(char *addr, int size)
{
    munmap(addr, size);
}

/*-------------------------------------------------------------------------*/

/* mesh_init_from_memory :                                                 */

/* Given an xml string buffer and the string size size returns the         */

/* corresponding mesh structure                                            */

/* - the xml parser need a 0 terminated string                             */

/*-------------------------------------------------------------------------*/
struct mesh_t *mesh_init_from_memory(char *buffer, int size, char *string)
{
    xmlDocPtr doc;

    char *buf;

    struct mesh_t *m;

    if (!buffer)
        return (NULL);

    /* force the buffer to be 0 terminated */
    buf = (char *) malloc((size + 1) * sizeof(char));
    memcpy(buf, buffer, size);
    buf[size] = '\0';

    doc = xmlParseMemory(buf, size);
    if (doc == NULL) {
        fprintf(stderr,
                "(%s):mesh_init_from_memory : could not parse string [%*s], passed size was %d\n",
                __FILE__, size, buf, size);
        xmlFreeDoc(doc);
        return (NULL);
    }
    if (xmlDocGetRootElement(doc) == NULL) {
        fprintf(stderr, "(%s): mesh_init_from_file : string is empty\n",
                __FILE__);
        xmlFreeDoc(doc);
        return (NULL);
    }
    free(buf);
    m = mesh_init_parsing(doc, string);
    xmlFreeDoc(doc);
    return (m);
}

/**\brief Return an XML fragment with the <data> tag.
 *
 * Given a filename (full pathname), the number of files
 * to be generated and the output format
 * return a string corresponding to the
 * xml fragment that specify the data filenames
 * constructed from <filename> :
 * <filename>-0, <filename>-1, ...
 *
 * E.g. for
 * ray2mesh -o sliced/sliced-1-1-r2m with mpirun -np 2 MPI processes
 * will output :
 * <data format="r2m" dir="sliced">
 *     <file name="sliced-1-1-r2m-1"/>
 *     <file name="sliced-1-1-r2m-2"/>
 * </data>
 *
 * @param filename (full path)
 * @param number of elements in filenames
 * @param r2m boolean set to 1 if files are r2m formatted
 */
char *mesh_xml_data_string(char *filename, int nbfiles, int r2m)
{
    char *pathcopy1, *pathcopy2;

    char **filenames;

    char *basefilename;

    char *dir;

    char *buff = NULL;

    int lbase;

    int i;

    assert(nbfiles > 0);
    assert(filename);
    /* separate dirname from file basename */
    pathcopy1 = strdup(filename);
    assert(pathcopy1);
    pathcopy2 = strdup(filename);
    assert(pathcopy2);
    dir = dirname(pathcopy1);
    basefilename = basename(pathcopy2);
    filenames = (char **) malloc(nbfiles * sizeof(char *));
    assert(filenames);
    if (nbfiles == 1) {
        filenames[0] = strdup(basefilename);
        assert(filenames[0]);
    } else {
        for (i = 0; i < nbfiles; i++) {
            filenames[i] =
                malloc((strlen(basefilename) + 7) * sizeof(char));
            sprintf(filenames[i], "%s-%d", basefilename, i);
        }
    }
    /* compute size of buffer to allocate */
    lbase = strlen("< =\"r2m\" =\"\"> </>\n\n") + strlen(DATA_TAG);     /* <data> tag */
    for (i = 0; i < DATA_NPROPS; i++)
        lbase += strlen(DATA_PROPS[i]) + 1;
    for (i = 0; i < FILE_NPROPS; i++)
        lbase += strlen(FILE_PROPS[i]) + strlen(" =\"\" ") * nbfiles;
    for (i = 0; i < nbfiles; i++) {
        lbase +=
            strlen(filenames[i]) + strlen(FILE_TAG) +
            strlen("      < =" "/> \n");
    }
    buff = malloc(lbase * sizeof(char));
    sprintf(buff, "<%s %s=\"%s\" %s=\"%s\">\n",
            DATA_TAG, DATA_PROPS[0], r2m ? "r2m" : "sco", DATA_PROPS[1],
            dir);
    for (i = 0; i < nbfiles; i++) {
        sprintf(buff + strlen(buff),
                "      <%s %s=\"%s\"/>\n",
                FILE_TAG, FILE_PROPS[0], filenames[i]);
    }
    sprintf(buff + strlen(buff), "</%s>\n", DATA_TAG);
    /* free */
    for (i = 0; i < nbfiles; i++)
        free(filenames[i]);
    free(filenames);
    free(pathcopy1);
    free(pathcopy2);
    return (buff);
}

/**\brief parse a string with coma separated atoms.
 *
 * parse a string which has separated atoms and return a NULL
 * terminated array of sub-strings. The separator is a string.
 * @param str the string to parse.
 * @param sep the separator.
 */
char **parse_separated_list(char *str, char *sep)
{
    char **l = NULL;

    char *ptr;

    char *token;

    int nbelem;

    ptr = str;
    if (str) {
        nbelem = 0;
        token = strtok_r(str, sep, &ptr);
        do {
            nbelem++;
            l = (char **) realloc(l, nbelem * sizeof(char *));
            l[nbelem - 1] = strdup(token);
            assert(l[nbelem - 1]);
        } while ((token = strtok_r(NULL, ",", &ptr)));
        l = (char **) realloc(l, (nbelem + 1) * sizeof(char *));
        l[nbelem] = NULL;
    }
    return (l);
}
