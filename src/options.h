enum options {
    OPT_ZERO = 0,
    OPT_HELP,
    OPT_VERBOSE,
    OPT_DEBUG,
    OPT_MESHFILE,
    OPT_VTKFILE,
    OPT_IMPORTFILE,
    OPT_FORMAT,
    OPT_VTK_ALLPOINTS,
    OPT_BEGIN_LAYER,
    OPT_END_LAYER,
    OPT_USE_LOGSCALE,
    OPT_SLICEMODE,
    OPT_SHOW_XML_SECTION,
    OPT_VERSION
};

const struct poptOption options[] = {
    /* long optin,short option, type,var,default val,explanation,expl 2 */
    {
     "help", 'h', POPT_ARG_NONE,
     NULL, OPT_HELP,
     "help", "display this help message"},
    {
     "verbose", (char) NULL, POPT_ARG_NONE,
     &VERBOSE, OPT_VERBOSE,
     "Verbose Mode", NULL},
    {
     "show-xml-section", 0, POPT_ARG_NONE,
     &show_xml_section, OPT_SHOW_XML_SECTION,
     "Show XML section", NULL},
    {
     "debug", 'd', POPT_ARG_NONE,
     &DEBUG, OPT_DEBUG,
     "Debug Mode", NULL},
    {
     "meshfile", 'm', POPT_ARG_STRING,
     &conffilename, OPT_MESHFILE,
     "Use mesh config file (xml)", "FILE"},
    {
     "vtkfile", 'o', POPT_ARG_STRING,
     &vtkfilename, OPT_VTKFILE,
     "Export mesh and score data  to vtk file", "FILE"},
    {
     "importfile", 'i', POPT_ARG_STRING,
     &importfilename, OPT_IMPORTFILE,
     "Import data into mesh (comma separated list: -i f1.xml,f2.xml,...)",
     "FILE[,...,FILE]"},
    {
     "format", 'f', POPT_ARG_STRING,
     &format_opt, OPT_FORMAT,
     "data to import is 'sco', 'r2m' or 'irm'",
     "sco|r2m|irm"},
    {
     "vtk-all-points", 'a', POPT_ARG_NONE,
     NULL, OPT_VTK_ALLPOINTS,
     "Duplicate cells vertices", NULL},
    {
     "begin-layer", 'b', POPT_ARG_INT,
     &begin_layer, OPT_BEGIN_LAYER,
     "Layer number to begin from", "INT (>=0)"},
    {
     "end-layer", 'e', POPT_ARG_INT,
     &end_layer, OPT_END_LAYER,
     "Layer number to end at", "INT (>=0)"},
    {
     "use-log-scale", 'l', POPT_ARG_NONE,
     NULL, OPT_USE_LOGSCALE,
     "Use log scale when importing r2m files (default is NO)", NULL},
    {
     "version", 'v', POPT_ARG_NONE,
     NULL, OPT_VERSION,
     "Print version number", NULL},
    {NULL, '\0', 0, NULL, 0, NULL, NULL}
};
