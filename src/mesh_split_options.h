const struct poptOption split_options[] = {
    {
     "help", 'h', POPT_ARG_NONE,
     NULL, 1,
     "help", "display this help message"},
    {
     "mesh", 'm', POPT_ARG_STRING,
     &mopt, 2,
     "xml mesh description file", "string"},
    {
     "lat-div", 'l', POPT_ARG_INT,
     &lat_div_opt, 3,
     "number of equal angular divisions in latitude", "int"},
    {
     "lon-div", 'L', POPT_ARG_INT,
     &lon_div_opt, 4,
     "number of equal angular divisions in longitude", "int"},
    {
     "basename", 'b', POPT_ARG_STRING,
     &bname, 5,
     "basename of the xml files to generate", "string"},
    {
     "version", 'v', POPT_ARG_NONE,
     NULL, 10,
     "print version number", NULL},
    {NULL, '\0', 0, NULL, 0, NULL, NULL}
};
