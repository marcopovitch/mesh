SUBDIRS = doc share src 

 ## We dist autogen.sh since this is an example program
 ## Real-world programs do not need to distribute autogen.sh
#ACLOCAL_AMFLAGS = -I m4

EXTRA_DIST = TODO autogen.sh libmesh.pc.in

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libmesh.pc

dist-hook: libmesh.spec
	cp libmesh.spec $(distdir)

rpms-x86 : distcheck
	rpm -ta --target i586 @PACKAGE@-@VERSION@.tar.gz
rpms-alpha : distcheck
	rpm -ta @PACKAGE@-@VERSION@.tar.gz

