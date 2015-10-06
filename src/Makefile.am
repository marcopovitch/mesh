#
# Makefile for mesh sources (/src) 
#
# sur seven
#-----------
# la phase de link pose probeme :
# on peut utiliser :
# gcc -o mesh cell.o export_vtk.o main.o mesh.o point3d.o -L/usr/local/O2/lib -lglib -lpopt -lxml -lm ../intl/libintl.a
#
# for 64bits MacOSX
#------------------
# you need to rebuild libpopt and libxml2 with 64bits support and set 
# export  PKG_CONFIG_PATH=/where-are-those-libs/pkgconfig   
# libxml now is shipped with mac os x 10.5 (/usr/lib/pkgconfig/libxml-2.0.pc).
#
#-----------------------------------------
AM_LDFLAGS=`${pkgconfig_invoke} --libs libxml-2.0` \
	   `${pkgconfig_invoke} --libs popt` -lm
AM_CFLAGS = ${extra_inc} `${pkgconfig_invoke} --cflags libxml-2.0` \
	   `${pkgconfig_invoke} --cflags popt` @CFLAGS@ -g

LIBTOOL_DEPS = @LIBTOOL_DEPS@
libtool: $(LIBTOOL_DEPS)
	      $(SHELL) ./config.status --recheck

INCLUDES = -I$(top_srcdir) -I$(includedir)

common =	point3d.c point3d.h \
	        convert_coord.h convert_coord.c\
		cell.c cell.h \
		cellinfo.c cellinfo.h\
		meridian.c meridian.h \
		layer.c layer.h \
		mesh.c mesh.h\
		mesh_init.c\
		export_vtk.c export_vtk.h \
		export_r2m.c export_r2m.h \
		const.h options.h\
		modulo.c modulo.h \
		import.c import.h \
		mesh2xml.c mesh2xml.h\
		metacell.h metacell.c\
		extern.h 

lib_LTLIBRARIES=libmesh.la

libmesh_la_SOURCES = $(common)

##----- published headers files-------------------

libmeshincludedir=$(includedir)/mesh

libmeshinclude_HEADERS = mesh.h\
			 layer.h\
			 point3d.h\
			 export_vtk.h\
			 export_r2m.h\
			 const.h\
			 convert_coord.h\
			 cell.h\
       cellinfo.h\
			 import.h\
			 modulo.h\
			 mesh2xml.h\
			 metacell.h\
			 extern.h
##--- see end of file for libtool versionning explanations

libmesh_la_LDFLAGS = -version-info 1:0:0

##----utility mesh is shipped with library-----------

bin_PROGRAMS = mesh mesh_split testxml

mesh_SOURCES= main.c
mesh_CFLAGS = ${extra_inc} `${pkgconfig_invoke} --cflags libxml-2.0` \
	      `${pkgconfig_invoke} --cflags popt` @CFLAGS@
mesh_LDFLAGS = ${extra_ld} `${pkgconfig_invoke} --libs libxml-2.0` \
	       `${pkgconfig_invoke} --libs popt` -lm
#mesh_LDADD =  -lm -lpopt -lmesh ## to work on Irix
mesh_LDADD = -lmesh -lm -lpopt  -lxml2 ## to work on Irix


mesh_split_SOURCES= mesh_split.c mesh_split_options.h
mesh_split_CFLAGS = ${extra_inc} `${pkgconfig_invoke} --cflags libxml-2.0` \
		    `${pkgconfig_invoke} --cflags popt` @CFLAGS@
mesh_split_LDFLAGS = ${extra_ld} `${pkgconfig_invoke} --libs libxml-2.0` \
		     `${pkgconfig_invoke} --libs popt` -lm
mesh_split_LDADD =  -lmesh -lm -lpopt -lxml2 ## to work on Irix

noinst_PROGRAMS = testxml
testxml_SOURCES = testxml.c

testxml_CFLAGS = ${mesh_CFLAGS}
testxml_LDFLAGS = ${mesh_LDFLAGS}
testxml_LDADD = ${mesh_LDADD}

##-------- Notes from `info libtool`----------------------
## 
## libmesh_la_LDFLAGS = -version-info CURRENT:REVISION:AGE
##
## CURRENT: The most recent interface number that this library implements.
## REVISION The implementation number of the CURRENT interface.
## AGE The difference between the newest and oldest interfaces that this
##     library implements.  In other words, the library implements all the
##    interface numbers in the range from number `CURRENT - AGE' to
##	`CURRENT'.
##
##
##  * If the library source code has changed at all since the last
##      update, then increment REVISION (`C:R:A' becomes `C:r+1:A').
##
##  * If any interfaces have been added, removed, or changed since the
##       last update, increment CURRENT, and set REVISION to 0.
##
##  * If any interfaces have been added since the last public release,
##       then increment AGE.
##
##  * If any interfaces have been removed since the last public release,
##       then set AGE to 0.

