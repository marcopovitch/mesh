#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "mesh.h"
#include "mesh2xml.h"

#include "import.h"
#include "metacell.h"

int main(int argc, char **argv)
{
    int xml_buffer_size;
    char *xml_buffer;
    struct mesh_t *mesh;
    int i;

    if (argc != 2) {
        printf
            ("Checks a mesh configuration XML file and associated functions of libmesh\nUsage : %s <xml-file>\n",
             argv[0]);
        exit(1);
    }

        /*--------------------- mesh_init_from_file() ------------------*/
    printf("* testing mesh_init_from_file(%s) ... ", argv[1]);
    fflush(stdout);
    if (!mesh_init_from_file(argv[1])) {
        printf("failed !\n");
        exit(1);
    }
    printf("OK.\n");

        /*--------------------- mesh_init_from_memory() ------------------*/
    printf("* testing load_file_to_memory(%s) ... ", argv[1]);
    if ((xml_buffer =
         load_file_to_memory(argv[1], &xml_buffer_size)) == NULL) {
        printf("failed !\n");
        exit(1);
    }
    printf("OK.\n");

    printf("* testing mesh_init_from_memory(%s) ... ", argv[1]);
    fflush(stdout);
    if (!mesh_init_from_file(argv[1])) {
        printf("failed !\n");
        exit(1);
    }
    printf("OK.\n");
    unload_file_from_memory(xml_buffer, xml_buffer_size);

    /* shown mesh sections */
    mesh = mesh_init_from_file(argv[1]);
    assert(mesh);
    mesh_show_sections(mesh);

    /* rewrite the xml */
    mesh2xml(mesh, "test.xml");

    /* mesh construction */
    make_mesh(mesh);

    /* import irm files */
    if (mesh->data[IRM]) {
        for (i = 0; i < mesh->data[IRM]->ndatafile; i++) {
            fprintf(stderr, "'%s'\n", mesh->data[IRM]->filename[i]);
            import2mesh_irm_file(mesh, mesh->data[IRM]->filename[i], NULL);
        }
        metacell_find_neighbourhood(mesh);
    } else {
        fprintf(stdout, "No irm section\n");
    }

    /* import sco files */
    if (mesh->data[SCO]) {
        for (i = 0; mesh->data[SCO] && i < mesh->data[SCO]->ndatafile; i++) {
            import2mesh_sco_file(mesh, mesh->data[SCO]->filename[i], 0, /* use log scale */
                                 NULL);
        }
    } else {
        fprintf(stdout, "No sco section\n");
    }

    return (0);
}
