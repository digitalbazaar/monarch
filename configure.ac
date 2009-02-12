# Configure script for Digital Bazaar common source distribution
# Usage: Run ./configure once 
# Author: Manu Sporny

AC_INIT([dbcore],[3.1],[support@digitalbazaar.com])
AC_CONFIG_AUX_DIR(setup)

# Setup standard build environment variables
DBCOREDIR=`pwd`
# Note: 'date -R' not portable to MacOS
DATE_RFC_2822=`date +"%a, %d %b %Y %H:%M:%S %z"`
DATE_YMD=`date +%Y%m%d`
FULL_VERSION=${PACKAGE_VERSION}-${DATE_YMD}

# Check for particular build options
AC_ARG_ENABLE([linux],
   AS_HELP_STRING([--enable-linux], [compile for linux [[yes]]]),
   [
      case "${enableval}" in
         yes) BUILD_FOR_LINUX=yes ;;
         no) BUILD_FOR_LINUX=no ;;
         *) AC_MSG_ERROR(bad value ${enableval} for --enable-linux) ;;
      esac
   ], [BUILD_FOR_LINUX=yes])  dnl Default value

AC_ARG_ENABLE([windows],
   AS_HELP_STRING([--enable-windows], [compile for windows [[no]]]),
   [
      case "${enableval}" in
         yes) BUILD_FOR_WINDOWS=yes ;;
         no) BUILD_FOR_WINDOWS=no ;;
         *) AC_MSG_ERROR(bad value ${enableval} for --enable-windows) ;;
      esac
   ], [BUILD_FOR_WINDOWS=no])  dnl Default value

AC_ARG_ENABLE([macos],
   AS_HELP_STRING([--enable-macos], [compile for MacOS [[no]]]),
   [
      case "${enableval}" in
         yes) BUILD_FOR_MACOS=yes ;;
         no) BUILD_FOR_MACOS=no ;;
         *) AC_MSG_ERROR(bad value ${enableval} for --enable-macos) ;;
      esac
   ], [BUILD_FOR_MACOS=no])  dnl Default value

echo -e "\n--------- Configuring DB Core Build Environment -----------"

# Check the system type
AC_CANONICAL_BUILD()

# Checking for standard build tools
#AC_LANG(C++)
AC_PROG_INSTALL()

# Conditional setup for Linux build
if test "$BUILD_FOR_LINUX" = "yes" ; then
   AC_PROG_CXX()
   AC_CHECK_PROG(LINUX_AR, ar, ar, not found)
   LINUX_CXX="$CXX"
fi

# Conditional setup for Windows build
if test "$BUILD_FOR_WINDOWS" = "yes" ; then
   AC_CHECK_PROG(WINDOWS_CXX, i586-mingw32msvc-g++, 
                 i586-mingw32msvc-g++, not found)
   AC_CHECK_PROG(WINDOWS_AR, i586-mingw32msvc-ar, 
                 i586-mingw32msvc-ar, not found)
   AC_CHECK_PROG(WINDOWS_DLLWRAP, i586-mingw32msvc-dllwrap,
                 i586-mingw32msvc-dllwrap, not found)
   AC_CHECK_PROG(WINDOWS_AS, i586-mingw32msvc-as,
                 i586-mingw32msvc-as, not found)
   # If the mingw32 compiler wasn't found, use the system default
   if test "$WINDOWS_CXX" = "not found" ; then
      AC_PROG_CXX()
      WINDOWS_CXX="$CXX"
   fi
fi

# Conditional setup for MacOS build
if test "$BUILD_FOR_MACOS" = "yes" ; then
   AC_PROG_CXX(g++-4 g++)
   AC_CHECK_PROG(MACOS_AR, ar, ar, not found)
   AC_CHECK_PROG(MACOS_AS, as, as, not found)
   MACOS_CXX="$CXX"
fi

AC_ARG_ENABLE([sphinx],
   AS_HELP_STRING([--disable-sphinx], [disable sphinx search [[no]]]),
   [
      case "${enableval}" in
         yes) BUILD_SPHINX=yes ;;
         no) BUILD_SPHINX=no ;;
         *) AC_MSG_ERROR(bad value ${enableval} for --enable-sphinx) ;;
      esac
   ], [BUILD_SPHINX=yes])  dnl Default value
AC_SUBST(BUILD_SPHINX)

# FIXME: Perform compilation environment tests
#AC_CHECK_HEADERS(iostream)
#AC_CHECK_HEADERS(openssl/opensslv) 
#AC_CHECK_HEADERS(expat)
#AC_CHECK_HEADERS(sqlite3)
#AC_CHECK_HEADERS(mysql/mysql)

# FIXME: Make sure the proper libraries exist
#AC_CHECK_LIB(pthread, main, [], AC_MSG_ERROR(could not find pthread library))
#AC_CHECK_LIB(crypto, main, [], AC_MSG_ERROR(could not find crypto library))
#AC_CHECK_LIB(ssl, main, [], AC_MSG_ERROR(could not find ssl library))
#AC_CHECK_LIB(expat, main, [], AC_MSG_ERROR(could not find expat library))
#AC_CHECK_LIB(sqlite3, main, [], AC_MSG_ERROR(could not find sqlite3 library))

# allow explicit disable of MySQL support
AC_ARG_ENABLE([mysql],
   AS_HELP_STRING([--disable-mysql], [disable MySQL [[no]]]),
   [
      case "${enableval}" in
         yes) BUILD_MYSQL=yes ;;
         no) BUILD_MYSQL=no ;;
         *) AC_MSG_ERROR(bad value ${enableval} for --enable-mysql) ;;
      esac
   ], [BUILD_MYSQL=yes])  dnl Default pre lib detection value
AC_SUBST(BUILD_MYSQL)

# if MySQL support enabled, check for the lib
if test "x$BUILD_MYSQL" = "xyes"; then
   AC_CHECK_LIB(mysqlclient, mysql_real_connect,
     [:],
     [AC_MSG_ERROR(mysqlclient library not found)]
   )
fi

# Make all the proper substitutions
AC_SUBST(DBCOREDIR)

AC_SUBST(DATE_RFC_2822)
AC_SUBST(FULL_VERSION)

AC_SUBST(BUILD_FOR_LINUX)
AC_SUBST(BUILD_FOR_WINDOWS)
AC_SUBST(BUILD_FOR_MACOS)

AC_SUBST(LINUX_CXX)
AC_SUBST(WINDOWS_CXX)
AC_SUBST(MACOS_CXX)

AC_SUBST(LINUX_AR)
AC_SUBST(WINDOWS_AR)
AC_SUBST(MACOS_AR)

AC_SUBST(WINDOWS_AS)
AC_SUBST(WINDOWS_DLLWRAP)
AC_SUBST(MACOS_AS)

AC_SUBST(JAVADOC)

# Generating files
AC_CONFIG_FILES([
          3rdparty/cpp/Makefile
          Makefile
          app/cpp/Makefile
          compress/cpp/Makefile
          config/cpp/Makefile
          crypto/cpp/Makefile
          data/cpp/Makefile
          event/cpp/Makefile
          fiber/cpp/Makefile
          io/cpp/Makefile
          logging/cpp/Makefile
          mail/cpp/Makefile
          modest/cpp/Makefile
          net/cpp/Makefile
          rt/cpp/Makefile
          setup/Makefile.base
          setup/docs.doxygen
          sphinx/cpp/Makefile
          sql/cpp/Makefile
          sqlite3/cpp/Makefile
          mysql/cpp/Makefile
          test/cpp/Makefile
          util/cpp/Makefile
          validation/cpp/Makefile
])

# Generate the platform-specific build file
if test "$BUILD_FOR_LINUX" = "yes" ; then
AC_CONFIG_FILES([ setup/Makefile.linux ])
fi
if test "$BUILD_FOR_MACOS" = "yes" ; then
AC_CONFIG_FILES([ setup/Makefile.macos ])
fi
if test "$BUILD_FOR_WINDOWS" = "yes" ; then
AC_CONFIG_FILES([ setup/Makefile.mingw32 ])
fi

# Generate scripts and configuration files for release
AC_CONFIG_FILES([
	installers/debian/changelog
	setup/capfile
])
AC_CONFIG_FILES([dbcore-run], [chmod +x dbcore-run])
AC_CONFIG_FILES([installers/debian/rules], [chmod +x installers/debian/rules])

AC_OUTPUT

# Dump the build configuration
echo -e "\n--------- DB Core Build Environment -----------"
echo "System            : $PACKAGE_NAME $FULL_VERSION"
echo "Build system      : $build_cpu-$build_vendor-$build_os"
if test "x$BUILD_FOR_LINUX" = "xyes"; then
   echo "Linux             : enabled"
   echo "Linux C++ compiler: $LINUX_CXX"
   echo "Linux C++ archiver: $LINUX_AR"
else
   echo "Linux             : disabled (use --enable-linux to enable)"
fi
if test "x$BUILD_FOR_WINDOWS" = "xyes"; then
   echo "Win32             : enabled"
   echo "Win32 C++ compiler: $WINDOWS_CXX"
   echo "Win32 C++ archiver: $WINDOWS_AR"
   echo "Win32 assembler   : $WINDOWS_AS"
   echo "Win32 DLL wrapper : $WINDOWS_DLLWRAP"
else
   echo "Win32             : disabled (use --enable-windows to enable)"
fi
if test "x$BUILD_FOR_MACOS" = "xyes"; then
   echo "MacOS             : enabled"
   echo "MacOS C++ compiler: $MACOS_CXX"
   echo "MacOS C++ archiver: $MACOS_AR"
   echo "MacOS assembler   : $MACOS_AS"
else
   echo "MacOS             : disabled (use --enable-macos to enable)"
fi

if test "x$BUILD_MYSQL" = "xyes"; then
   echo "MySQL             : enabled"
else
   echo "MySQL             : disabled"
fi

if test "x$BUILD_SPHINX" = "xyes"; then
   echo "Sphinx            : enabled"
else
   echo "Sphinx            : disabled"
fi

