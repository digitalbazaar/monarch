# Configure script for Digital Bazaar common source distribution
# Usage: Run ./configure once 
# Author: Manu Sporny

AC_PREREQ([2.60])
AC_INIT([monarch],[3.2.3],[support@digitalbazaar.com])
AC_CONFIG_AUX_DIR(setup)
AC_CONFIG_MACRO_DIR(setup)

# Versioning
MONARCH_VERSION_MAJOR=3
MONARCH_VERSION_MINOR=2
MONARCH_VERSION_MICRO=3
# Note: 'date -R' not portable to MacOS
DATE_RFC_2822=`date +"%a, %d %b %Y %H:%M:%S %z"`
DATE_YMD=`date +%Y%m%d`
FULL_DOTTED_VERSION=${PACKAGE_VERSION}
FULL_VERSION=${PACKAGE_VERSION}-${DATE_YMD}

# Setup standard build environment variables
MONARCH_DIR=`pwd`

# Check for particular build options
AC_ARG_ENABLE([linux],
   AS_HELP_STRING([--enable-linux], [compile for linux [[yes]]]),
   [
      case "${enableval}" in
         yes) BUILD_FOR_LINUX=yes 
              BUILD_FOR_WINDOWS=no
              BUILD_FOR_MACOS=no
              ;;
         no) BUILD_FOR_LINUX=no ;;
         *) AC_MSG_ERROR(bad value ${enableval} for --enable-linux) ;;
      esac
   ], [BUILD_FOR_LINUX=yes])  dnl Default value

AC_ARG_ENABLE([windows],
   AS_HELP_STRING([--enable-windows], [compile for windows [[no]]]),
   [
      case "${enableval}" in
         yes) BUILD_FOR_WINDOWS=yes 
              BUILD_FOR_LINUX=no
              BUILD_FOR_MACOS=no
              ;;
         no) BUILD_FOR_WINDOWS=no ;;
         *) AC_MSG_ERROR(bad value ${enableval} for --enable-windows) ;;
      esac
   ], [BUILD_FOR_WINDOWS=no])  dnl Default value

AC_ARG_ENABLE([macos],
   AS_HELP_STRING([--enable-macos], [compile for MacOS [[no]]]),
   [
      case "${enableval}" in
         yes) BUILD_FOR_MACOS=yes 
              BUILD_FOR_LINUX=no
              BUILD_FOR_WINDOWS=no
              ;;
         no) BUILD_FOR_MACOS=no ;;
         *) AC_MSG_ERROR(bad value ${enableval} for --enable-macos) ;;
      esac
   ], [BUILD_FOR_MACOS=no])  dnl Default value

echo -e "\n--------- Configuring Monarch Build Environment -----------"

# Setup for pkg-config
PKG_PROG_PKG_CONFIG

# Check the system type
AC_CANONICAL_BUILD()

# Checking for standard build tools
#AC_LANG(C++)
AC_PROG_INSTALL()
AC_CHECK_PROGS(PYTHON, python2.5 python2.4, not found)

# Conditional setup for Linux build
if test "$BUILD_FOR_LINUX" = "yes" ; then
   AC_PROG_CC()
   AC_PROG_CXX()
   AC_CHECK_PROG(AR, ar, ar, not found)
   AC_CHECK_PROG(AS, as, as, not found)
   PLATFORM=linux
   LIB_PREFIX=lib
   DYNAMIC_LIB_EXT=so
   STATIC_LIB_EXT=a
   EXECUTABLE_EXT=
fi

# Conditional setup for Windows build
if test "$BUILD_FOR_WINDOWS" = "yes" ; then
   AC_CHECK_PROG(CXX, i586-mingw32msvc-g++,
                 i586-mingw32msvc-g++, not found)
   AC_CHECK_PROG(AR, i586-mingw32msvc-ar,
                 i586-mingw32msvc-ar, not found)
   AC_CHECK_PROG(AS, i586-mingw32msvc-as,
                 i586-mingw32msvc-as, not found)
   # If the mingw32 compiler wasn't found, use the system default
   if test "$CXX" = "not found" ; then
      AC_PROG_CXX()
   fi
   LDFLAGS="$LDFLAGS -Wl,--enable-auto-import"
   PLATFORM=windows
   LIB_PREFIX=
   DYNAMIC_LIB_EXT=dll
   STATIC_LIB_EXT=a
   EXECUTABLE_EXT=.exe
fi

# Conditional setup for MacOS build
if test "$BUILD_FOR_MACOS" = "yes" ; then
   AC_PROG_CC(gcc-4 gcc)
   AC_PROG_CXX(g++-4 g++)
   AC_CHECK_PROG(AR, ar, ar, not found)
   AC_CHECK_PROG(AS, as, as, not found)
   LDFLAGS="$LDFLAGS -L/sw/lib"
   PLATFORM=macos
   LIB_PREFIX=lib
   DYNAMIC_LIB_EXT=dylib
   STATIC_LIB_EXT=a
   EXECUTABLE_EXT=
   if test -d /sw/include; then
      CPPFLAGS="$CPPFLAGS -I/sw/include"
   fi
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
# Disable sphinx support in Windows
if test "x$BUILD_FOR_WINDOWS" = "xyes"; then
   BUILD_SPHINX=no
fi
AC_SUBST(BUILD_SPHINX)

# FIXME: Perform compilation environment tests 
#AC_CHECK_HEADERS(iostream)

# Make sure the proper libraries exist
# Windows has special bundled support

if test "$BUILD_FOR_WINDOWS" = "no" ; then

   ## Check for pkg-config enabled libs

   PKG_CHECK_MODULES([CRYPTO], [libcrypto])
   PKG_CHECK_MODULES([SSL], [libssl])
   PKG_CHECK_MODULES([SQLITE3], [sqlite3 >= 3.6.17])
   # FIXME: pkg-config sets _LIBS/_CFLAGS that could be used in Makefiles

   ## Check for pthread support
   
   AC_CHECK_LIB([pthread], [pthread_create], [])
   
   ## Check for expat support
   
# FIXME: header check not working on MacOS
#   AC_CHECK_LIB([expat], XML_ParserCreate,
#      [AC_CHECK_HEADERS([expat.h], [have_expat=true], [have_expat=false])],
#      [have_expat=false])
   AC_CHECK_LIB([expat], XML_ParserCreate,
      [have_expat=true],
      [have_expat=false])
   
   if ! $have_expat; then
      AC_MSG_ERROR([You need the eXpat xml parser]
         [http://expat.sourceforge.net/])
   fi

fi

## Control and check mysql support

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

# Disable mysql support in Windows
if test "x$BUILD_FOR_WINDOWS" = "xyes"; then
   BUILD_MYSQL=no
fi
AC_SUBST(BUILD_MYSQL)

# if MySQL support enabled, check for the lib
if test "x$BUILD_MYSQL" = "xyes"; then
   AC_CHECK_LIB(mysqlclient, mysql_real_connect,
     [have_mysql=true], [have_mysql=false])
   if ! $have_mysql; then
      AC_MSG_ERROR(mysqlclient library not found)
   fi

   AC_CHECK_HEADER(mysql/mysql.h, [have_mysql=true], [have_mysql=false])
   if ! $have_mysql; then
      AC_MSG_ERROR(mysql/mysql.h header not found)
   fi
fi

dnl ----------------- disable test building and running -----------------

AC_ARG_ENABLE([tests],
   AC_HELP_STRING([--disable-tests], [disable building test apps [[no]]]),
   [
   case "${enableval}" in
      yes) BUILD_TESTS=yes; BUILD_TESTS_SET=yes ;;
      no)  BUILD_TESTS=no ;;
      *)   AC_MSG_ERROR(bad value ${enableval} for --disable-tests) ;;
   esac
   ],
   [BUILD_TESTS=yes]) dnl Default value
# Disable test building Windows
if test "x$BUILD_FOR_WINDOWS" = "xyes" -a "x$BUILD_TESTS_SET" != "xyes"; then
   BUILD_TESTS=no
fi
AC_SUBST(BUILD_TESTS)

dnl ----------------- disable log line numbers -----------------

AC_ARG_ENABLE([log-line-numbers],
   AC_HELP_STRING(
      [--disable-log-line-numbers],
      [disable log line numbers [[no]]]),
   [
   case "${enableval}" in
      yes) MO_DISABLE_LOG_LINE_NUMBERS=no ;;
      no)  MO_DISABLE_LOG_LINE_NUMBERS=yes ;;
      *)   AC_MSG_ERROR(bad value ${enableval} for --enable-log-line-numbers) ;;
   esac
   ],
   [MO_DISABLE_LOG_LINE_NUMBERS=no]) dnl Default value
if test "x$MO_DISABLE_LOG_LINE_NUMBERS" = xyes; then
   AC_DEFINE(MO_DISABLE_LOG_LINE_NUMBERS, 1,
      [Enable log line numbers.])
fi

dnl ----------------- optimization -----------------

dnl option to override default c++ optimization (often -g -O2)
AC_ARG_WITH(opt-flags,
  AC_HELP_STRING([--with-opt-flags=FLAGS],
    [use FLAGS for optimization (-O -fno-exceptions)]),
  [MO_CXX_OPT_FLAGS="${withval}"],
  [MO_CXX_OPT_FLAGS="-O -fno-exceptions"]) dnl Default value

AC_MSG_NOTICE(Using optimization flags: "$MO_CXX_OPT_FLAGS")

dnl ----------------- DynamicObject statistics -----------------

dnl low level DynamicObject statistics

AC_ARG_ENABLE([dyno-counts],
   AS_HELP_STRING([--enable-dyno-counts],
      [enable DynamicObject type counts [[no]]]),
   [
      case "${enableval}" in
         yes) MO_DYNO_COUNTS="-DMO_DYNO_COUNTS" ;;
         no) ;;
         *) AC_MSG_ERROR(bad value ${enableval} for --enable-dyno-counts) ;;
      esac
   ], [])  dnl Default value

AC_ARG_ENABLE([dyno-key-counts],
   AS_HELP_STRING([--enable-dyno-key-counts],
      [enable DynamicObject Map key counts [[no]]]),
   [
      case "${enableval}" in
         yes) MO_DYNO_KEY_COUNTS="-DMO_DYNO_KEY_COUNTS" ;;
         no) ;;
         *) AC_MSG_ERROR(bad value ${enableval} for --enable-dyno-key-counts) ;;
      esac
   ], [])  dnl Default value

MO_DEBUG_FLAGS="$MO_DYNO_COUNTS $MO_DYNO_KEY_COUNTS"
AC_MSG_NOTICE(Using debug flags: "$MO_DEBUG_FLAGS")

dnl ----------------- flags -----------------

CXXFLAGS="$CXXFLAGS $MO_CXX_OPT_FLAGS $MO_DEBUG_FLAGS"

dnl ----------------- substitutions -----------------

# Make all the proper substitutions
AC_SUBST(PLATFORM)
AC_SUBST(MONARCH_DIR)

AC_SUBST(MONARCH_VERSION_MAJOR)
AC_SUBST(MONARCH_VERSION_MINOR)
AC_SUBST(MONARCH_VERSION_MICRO)
AC_SUBST(DATE_RFC_2822)
AC_SUBST(FULL_DOTTED_VERSION)
AC_SUBST(FULL_VERSION)

AC_SUBST(BUILD_FOR_LINUX)
AC_SUBST(BUILD_FOR_WINDOWS)
AC_SUBST(BUILD_FOR_MACOS)

# Platform variables
AC_SUBST(LIB_PREFIX)
AC_SUBST(DYNAMIC_LIB_EXT)
AC_SUBST(STATIC_LIB_EXT)
AC_SUBST(EXECUTABLE_EXT)

AC_SUBST(PYTHON)

# Generating files
AC_CONFIG_FILES([
   .gitignore
   Makefile
   setup/Makefile.base
   setup/docs.doxygen
   configs/apps/test.config
   cpp/3rdparty/Makefile
   cpp/app/Makefile
   cpp/apps/monarch/Makefile
   cpp/apps/portmap/Makefile
   cpp/apps/tester/Makefile
   cpp/compress/Makefile
   cpp/config/Makefile
   cpp/crypto/Makefile
   cpp/data/Makefile
   cpp/event/Makefile
   cpp/fiber/Makefile
   cpp/http/Makefile
   cpp/io/Makefile
   cpp/kernel/Makefile
   cpp/logging/Makefile
   cpp/mail/Makefile
   cpp/modest/Makefile
   cpp/mysql/Makefile
   cpp/net/Makefile
   cpp/rt/Makefile
   cpp/sphinx/Makefile
   cpp/sql/Makefile
   cpp/sqlite3/Makefile
   cpp/test/Makefile
   cpp/tests/Makefile
   cpp/upnp/Makefile
   cpp/util/Makefile
   cpp/validation/Makefile
])

# Generate scripts and configuration files for release
AC_CONFIG_FILES([
   installers/debian/changelog
])
AC_CONFIG_FILES([monarch-run], [chmod +x monarch-run])
AC_CONFIG_FILES([cpp/tests/run-unit-tests.py], [chmod +x cpp/tests/run-unit-tests.py])
AC_CONFIG_FILES([installers/debian/rules], [chmod +x installers/debian/rules])

# add newlines to internal output file list
CONFIGURE_GENERATED_FILES="`echo $ac_config_files | tr ' ' '\n'`"
AC_SUBST(CONFIGURE_GENERATED_FILES)

AC_OUTPUT

# Dump the build configuration
echo -e "\n--------- Monarch Build Environment -----------"
echo "System            : $PACKAGE_NAME $FULL_VERSION"
echo "Build system      : $build_cpu-$build_vendor-$build_os"
echo "Build platform    : $PLATFORM"

if test "x$BUILD_FOR_LINUX" = "xyes"; then
   echo "Linux             : enabled"
else
   echo "Linux             : disabled (use --enable-linux to enable)"
fi
if test "x$BUILD_FOR_WINDOWS" = "xyes"; then
   echo "Win32             : enabled"
else
   echo "Win32             : disabled (use --enable-windows to enable)"
fi
if test "x$BUILD_FOR_MACOS" = "xyes"; then
   echo "MacOS             : enabled"
else
   echo "MacOS             : disabled (use --enable-macos to enable)"
fi
echo "C++ compiler      : $CXX"
echo "C++ archiver      : $AR"
echo "Assembler         : $AS"

echo "Optimization Flags: $MO_CXX_OPT_FLAGS"
echo "Debug Flags       : $MO_DEBUG_FLAGS"

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

if test "x$BUILD_TESTS" = "xyes"; then
   echo "Tests             : enabled"
else
   echo "Tests             : disabled (use --enable-tests to enable)"
fi

if test "x$MO_DISABLE_LOG_LINE_NUMBERS" = "xyes"; then
   echo "Log line numbers  : disabled (use --enable-log-line-numbers to enable)"
else
   echo "Log line numbers  : enabled"
fi
