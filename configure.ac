dnl Process this file with autoconf to produce a configure script.

AC_INIT([libmesh], [1.0pre9])
AM_INIT_AUTOMAKE
AC_PROG_CC
AC_PROG_INSTALL
AM_PROG_LIBTOOL
AC_SUBST(LIBTOOL_DEPS)
AC_CONFIG_MACRO_DIR([m4])


pkgconfig_invoke="pkg-config"
case "${host}" in
   *apple*)
          syst="OsX"
          extra_inc="${extra_inc} -Wall"
          pkgconfig_invoke="pkg-config"
   ;;
   i[[3456]]86-*-linux-gnu*)
          syst="Linux"
          extra_inc="${extra_inc} -Wall"
          pkgconfig_invoke="pkg-config"
   ;;
   mip*-*-irix*)
          syst="Irix"
          extra_inc="${extra_inc} -I/usr/freeware/include"
          extra_ld="${extra_ld} -L/usr/freeware/lib32"
          pkgconfig_invoke="pkg-config --define-variable=ROOT=/ --define-variable=ABILIB-lib32=lib32"
   ;;
esac

libxml2_inc=`${pkgconfig_invoke} --cflags libxml-2.0`
libxml2_ld=`${pkgconfig_invoke} --libs libxml-2.0`
popt_inc=`${pkgconfig_invoke} --cflags popt`
popt_ld=`${pkgconfig_invoke} --libs popt`
popt_inc=`${pkgconfig_invoke} --cflags popt`
extra_inc="${extra_inc} ${libxml2_inc}"
extra_ld="${extra_ld} ${libxml2_ld}"


dnl ---------------- Documentation generation ----------------------------------
DOC_GENERATOR=true ## later write a macro to see if doxygen is there

dnl Let people disable the doxygen doc generation.
AC_ARG_ENABLE(doc, [  --enable-doc  Use doc to build doxygen documentation [default=no]], enable_doc="$enableval", enable_doc=no)

if test x$enable_doc = xyes ; then
   if test x$DOC_GENERATOR = xtrue ; then
      enable_doc=yes
   else
      enable_doc=no
   fi
fi

AM_CONDITIONAL(ENABLE_DOC, test x$enable_doc = xyes)
AC_SUBST(extra_inc)
AC_SUBST(extra_ld)
AC_SUBST(pkgconfig_invoke)

echo "--------------------------Summary-----------------------------"
echo " System                : ${syst}"
echo " C compiler invocation : ${CC}                       "
echo " C Flags               : ${CFLAGS} ${extra_inc}      "
echo " LD flags              : ${LD_ADD} ${extra_ld}       "
echo " defines               : ${extra_defines}                 "
echo " pk-config invocation  : ${pkgconfig_invoke}              "
echo " Documentation build   : ${enable_doc}                    "
echo "--------------------------------------------------------------"

AC_OUTPUT([
Makefile
src/Makefile
share/Makefile
doc/Makefile
doc/html/Makefile
libmesh.spec
libmesh.pc
])
