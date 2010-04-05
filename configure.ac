# Configure script for Digital Bazaar common source distribution
# Usage: Run ./configure once 
# Author: Manu Sporny

AC_INIT([monarch],[3.2.1],[support@digitalbazaar.com])
AC_CONFIG_AUX_DIR(setup)

# Versioning
MONARCH_VERSION_MAJOR=3
MONARCH_VERSION_MINOR=2
MONARCH_VERSION_MICRO=1
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

# Check the system type
AC_CANONICAL_BUILD()

# Checking for standard build tools
#AC_LANG(C++)
AC_PROG_INSTALL()
AC_CHECK_PROGS(PYTHON, python2.5 python2.4, not found)

# Conditional setup for Linux build
if test "$BUILD_FOR_LINUX" = "yes" ; then
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
   AC_PROG_CXX(g++-4 g++)
   AC_CHECK_PROG(AR, ar, ar, not found)
   AC_CHECK_PROG(AS, as, as, not found)
   LDFLAGS="$LDFLAGS -L/sw/lib"
   PLATFORM=macos
   LIB_PREFIX=lib
   DYNAMIC_LIB_EXT=dylib
   STATIC_LIB_EXT=a
   EXECUTABLE_EXT=
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

# Disable mysql support in Windows
if test "x$BUILD_FOR_WINDOWS" = "xyes"; then
   BUILD_MYSQL=no
fi
AC_SUBST(BUILD_MYSQL)

# if MySQL support enabled, check for the lib
if test "x$BUILD_MYSQL" = "xyes"; then
   AC_CHECK_LIB(mysqlclient, mysql_real_connect,
     [:],
     [AC_MSG_ERROR(mysqlclient library not found)]
   )
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

AC_OUTPUT

# Write a proper .gitignore file based on files that are generated
GIT_IGNORE_FILES=".gitignore build dist autom4te.cache config.log config.status configure"
GIT_IGNORE_FILES="$GIT_IGNORE_FILES $ac_config_files"
echo $GIT_IGNORE_FILES | tr ' ' '\n' > .gitignore

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
