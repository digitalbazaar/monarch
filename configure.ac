# Configure script for Digital Bazaar common source distribution
# Usage: Run ./configure once 
# Author: Manu Sporny

AC_INIT(dbcore, 3.0, support@digitalbazaar.com)
AC_CONFIG_AUX_DIR(setup)

# Setup standard build environment variables
DBCOREDIR=`pwd`

# Check for particular build options
AC_ARG_ENABLE([linux],
               AC_HELP_STRING([--enable-linux],
                              [compile for linux (default is YES)]),
              [ac_enable_linux_build=yes], [ac_enable_linux_build=yes])

AC_ARG_ENABLE([windows],
               AC_HELP_STRING([--enable-windows],
                              [compile for windows (default is NO)]),
              [ac_enable_windows_build=yes], [ac_enable_windows_build=no])

echo -e "\n--------- Configuring DB Core Build Environment -----------"
# Check the system type
AC_CANONICAL_BUILD()

# Checking for standard build tools
#AC_LANG(C++)
AC_PROG_INSTALL()

# Check to see if we need to build for linux
if test "$ac_enable_linux_build" = "yes" ; then
   AC_PROG_CXX()
   AC_CHECK_PROG(LINUX_AR, ar, ar, not found)
   LINUX_CXX="$CXX"
   BUILD_FOR_LINUX="yes"
fi

# Check to see if we need to build for windows
if test "$ac_enable_windows_build" = "yes" ; then
   AC_CHECK_PROG(WINDOWS_CXX, i586-mingw32msvc-g++, 
                 i586-mingw32msvc-g++, not found)
   AC_CHECK_PROG(WINDOWS_AR, i586-mingw32msvc-ar, 
                 i586-mingw32msvc-ar, not found)

   # If the mingw32 compiler wasn't found, use the system default
   if test "$WINDOWS_CXX" = "not found" ; then
      AC_PROG_CXX()
      WINDOWS_CXX="$CXX"
   fi

   BUILD_FOR_WINDOWS="yes"
fi

# Perform compilation environment tests
#AC_CHECK_HEADERS(iostream)

# Make sure the proper libraries exist
#AC_CHECK_LIB(pthread, main, [], AC_MSG_ERROR(could not find pthread library))
#AC_CHECK_LIB(crypto, main, [], AC_MSG_ERROR(could not find crypto library))
#AC_CHECK_LIB(ssl, main, [], AC_MSG_ERROR(could not find ssl library))


echo -e "\n--------- DB Core Build Environment -----------"
echo "System            : $PACKAGE_STRING"
echo "Build system      : $build_cpu-$build_vendor-$build_os"
if test "$BUILD_FOR_LINUX" = "yes"; then
   echo "Linux C++ compiler: $LINUX_CXX"
   echo "Linux C++ archiver: $LINUX_AR"
fi
if test "$BUILD_FOR_WINDOWS" = "yes"; then
   echo "Win32 C++ compiler: $WINDOWS_CXX"
   echo "Win32 C++ archiver: $WINDOWS_AR"
fi

# Make all the proper substitutions
AC_SUBST(BUILD_FOR_LINUX)
AC_SUBST(BUILD_FOR_WINDOWS)

AC_SUBST(LINUX_CXX)
AC_SUBST(WINDOWS_CXX)

AC_SUBST(LINUX_AR)
AC_SUBST(WINDOWS_AR)

AC_SUBST(DBCOREDIR)
AC_SUBST(JAVADOC)


# Generating files
AC_OUTPUT(Makefile \
          setup/Makefile.base \
          config/cpp/Makefile \
          crypto/cpp/Makefile \
          data/cpp/Makefile \
          event/cpp/Makefile \
          io/cpp/Makefile \
          mail/cpp/Makefile \
          modest/cpp/Makefile \
          net/cpp/Makefile \
          rt/cpp/Makefile \
          sql/cpp/Makefile \
          test/cpp/Makefile \
          util/cpp/Makefile)
