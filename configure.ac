# this is example-file: configure.ac

# initial information about the project
AC_INIT([NwEpc],[0.16],[beta])

# check if the source folder is correct
AC_CONFIG_SRCDIR([nw-mme-emu/NwMmeMain.c])
AC_CONFIG_SRCDIR([nw-sae-gw/NwSaeGwMain.c])

# Checks for programs

# check for C++ preprocessor and compiler and the library compiler
AC_PROG_CXXCPP
AC_PROG_CXX
AC_PROG_RANLIB

# automake initialisation and check for minimal automake API version 1.9
AM_INIT_AUTOMAKE([1.9])

# use the C compiler for the following checks
AC_LANG([C])

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([string])
AC_CHECK_HEADERS([iostream])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T

# distribute additional compiler and linker flags
# --> set these variables instead of CXXFLAGS or LDFLAGS
AC_SUBST([AM_CFLAGS])
AC_SUBST([AM_LDFLAGS])

# files to generate via autotools (.am or .in source files)
AC_CONFIG_FILES([Makefile])
AC_CONFIG_FILES([nw-event/Makefile])
AC_CONFIG_FILES([nw-event/src/Makefile])
AC_CONFIG_FILES([nw-event/test-app/Makefile])
AC_CONFIG_FILES([nw-ipv4/Makefile])
AC_CONFIG_FILES([nw-ipv4/src/Makefile])
AC_CONFIG_FILES([nw-ipv4/test-app/Makefile])
AC_CONFIG_FILES([nw-ipv4/test-app/nw-helloworld/Makefile])
AC_CONFIG_FILES([nw-gre/Makefile])
AC_CONFIG_FILES([nw-gre/src/Makefile])
AC_CONFIG_FILES([nw-gre/test-app/Makefile])
AC_CONFIG_FILES([nw-gre/test-app/nw-helloworld/Makefile])
AC_CONFIG_FILES([nw-gtpv1u/Makefile])
AC_CONFIG_FILES([nw-gtpv1u/src/Makefile])
AC_CONFIG_FILES([nw-gtpv1u/test-app/Makefile])
AC_CONFIG_FILES([nw-gtpv1u/test-app/nw-helloworld/Makefile])
AC_CONFIG_FILES([nw-gtpv2c/Makefile])
AC_CONFIG_FILES([nw-gtpv2c/src/Makefile])
AC_CONFIG_FILES([nw-gtpv2c/test-app/Makefile])
AC_CONFIG_FILES([nw-gtpv2c/test-app/nw-helloworld/Makefile])
AC_CONFIG_FILES([nw-gtpv2c/test-app/nw-egtping/Makefile])
AC_CONFIG_FILES([nw-sdp/Makefile])
AC_CONFIG_FILES([nw-sdp/src/Makefile])
AC_CONFIG_FILES([nw-sdp/test-app/Makefile])
AC_CONFIG_FILES([nw-sdp/test-app/nw-helloworld/Makefile])
AC_CONFIG_FILES([nw-mme-emu/Makefile])
AC_CONFIG_FILES([nw-sae-gw/Makefile])

# generate the final Makefile etc.
AC_OUTPUT
