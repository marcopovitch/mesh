#include "mesh2xml.h"

void parameter2xml(FILE * fd, struct mesh_parameter_t *mp)
{
    float lonmin, lonmax;

    assert(mp);

    if (mp->lon_max > 360) {
        lonmax = fmod(mp->lon_max, 360.);
        lonmin = mp->lon_min - 360.;
    } else {
        lonmax = mp->lon_max;
        lonmin = mp->lon_min;
    }

    fprintf(fd,
            "<mesh \tlat-min=\"%.2f\" lat-max=\"%.2f\"\n\tlon-min=\"%.2f\" lon-max=\"%.2f\"\n\tlat-unit-size=\"%.2f\" lon-unit-size=\"%.2f\">\n\n",
            mp->lat_min, mp->lat_max,
            lonmin, lonmax, mp->lat_unit_size, mp->lon_unit_size);
}

void layer2xml(FILE * fd, struct mesh_t *m)
{
    int l;

    struct layer_t *layer;

    assert(m);

    fprintf(fd, "<model>\n");
    for (l = 0; l < m->nlayers; l++) {
        layer = m->layer[l];
        if (!layer)
            continue;
        fprintf(fd,
                "\t<layer name=\"%s\" zstart=\"%.2f\" zend=\"%.2f\" lat-unit=\"%d\" lon-unit=\"%d\" />\n",
                layer->name, layer->zstart, layer->zend, layer->lat_unit,
                layer->lon_unit);
    }

    fprintf(fd, "</model>\n\n");
}

void section_data2xml(FILE * fd, struct mesh_data_t *md)
{
    int i;

    assert(md);

    fprintf(fd, "<data format=\"%s\" directory=\"%s\">\n",
            md->format, md->directory);

    for (i = 0; i < md->ndatafile; i++) {
        fprintf(fd, "\t<file name=\"%s\"/>\n", md->filename[i]);
    }

    fprintf(fd, "</data>\n\n");
}

void mesh2xml(struct mesh_t *mesh, char *filename)
{
    FILE *fd;

    int f;

    assert(mesh);

    fprintf(stdout, "writing current mesh to xml file '%s'\n", filename);

    if (!(fd = fopen(filename, "w"))) {
        perror(filename);
        exit(1);
    }

    /* header2xml */
    fprintf(fd, "<?xml version=\"1.0\"?>\n");

    /* parameters */
    parameter2xml(fd, mesh->parameter);

    /* layers */
    layer2xml(fd, mesh);

    /* section data */
    for (f = 0; f < NB_MESH_FILE_FORMAT; f++) {
        if (!mesh->data[f]) {
            continue;
        }
        section_data2xml(fd, mesh->data[f]);
    }

    /* end */
    fprintf(fd, "</mesh>\n");
    fclose(fd);
}
