# Configure script for Digital Bazaar common source distribution
# Usage: Run ./configure once 
# Author: Manu Sporny

AC_PREREQ([2.60])
AC_INIT([monarch],[3.2.6],[support@digitalbazaar.com])
AC_CONFIG_AUX_DIR(setup)
AC_CONFIG_MACRO_DIR(setup)
AC_CONFIG_HEADERS([setup/config.h])

# Versioning
MONARCH_VERSION_MAJOR=3
MONARCH_VERSION_MINOR=2
MONARCH_VERSION_MICRO=6
# Note: 'date -R' not portable to MacOS
DATE_RFC_2822=`date +"%a, %d %b %Y %H:%M:%S %z"`
DATE_YMD=`date +%Y%m%d`
FULL_DOTTED_VERSION=${PACKAGE_VERSION}
FULL_VERSION=${PACKAGE_VERSION}-${DATE_YMD}

# Setup standard build environment variables
MONARCH_DIR=`pwd`

echo -e "\n--------- Configuring Monarch Build Environment -----------"

MO_MSG_CONFIG_BEGIN([General])
MO_MSG_CONFIG_OPTION([System], [$PACKAGE_NAME $FULL_VERSION])
MO_MSG_CONFIG_END

# Setup platform first
MO_PLATFORM

# Setup for pkg-config
PKG_PROG_PKG_CONFIG

# Checking for standard build tools
AC_LANG(C++)
AC_PROG_INSTALL()
AS_PATH_PYTHON([2.4])

# Common build options
MO_COMPILER

MO_MSG_CONFIG_BEGIN([General Options])
MO_MSG_CONFIG_OPTION_IF([Python],
   [test "x$PYTHON" != x],
   [$PYTHON (version $PYTHON_VERSION)], [])
MO_ARG_DEBUG
MO_ARG_OPT
MO_ARG_LOG_LINE_NUMBERS
MO_ARG_TESTS
MO_ARG_DOCS
MO_MSG_CONFIG_END

dnl ----------------- standard checks -----------------

# header checks
AC_CHECK_HEADERS([iostream])

dnl large file support
dnl include config.h where needed
AC_FUNC_FSEEKO
AC_SYS_LARGEFILE

dnl ----------------- common libraries -----------------

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

dnl ----------------- librdfa -----------------

# check for librdfa and headers
PKG_CHECK_MODULES([LIBRDFA], [librdfa], HAVE_LIBRDFA=yes, HAVE_LIBRDFA=no)

AC_SUBST(HAVE_LIBRDFA)
if test "x$HAVE_LIBRDFA" = "xyes"; then
   AC_DEFINE([HAVE_LIBRDFA], [1], ["librdfa support enabled"])
fi

dnl ----------------- MySQL -----------------

# allow explicit disable of MySQL support
AC_ARG_ENABLE([mysql],
   AS_HELP_STRING([--disable-mysql], [disable MySQL [no]]),
   [
      case "${enableval}" in
         yes) BUILD_MYSQL=yes ;;
         no) BUILD_MYSQL=no ;;
         *) AC_MSG_ERROR(bad value ${enableval} for --enable-mysql) ;;
      esac
   ], [BUILD_MYSQL=check])  dnl default

# if MySQL support requestd, check for support
if test "x$BUILD_MYSQL" = "xyes" -o "x$BUILD_MYSQL" = "xcheck"; then
   AC_PATH_PROGS([MYSQL_CONFIG], [mysql_config mysql_config5])
   if test "x$MYSQL_CONFIG" != "x"; then
      MYSQL_CFLAGS=`$MYSQL_CONFIG --cflags`
      MYSQL_LDFLAGS=`$MYSQL_CONFIG --libs`
      AC_SUBST(MYSQL_CFLAGS)
      AC_SUBST(MYSQL_LDFLAGS)
      BUILD_MYSQL=yes
   else
      if test "x$BUILD_MYSQL" = "xyes"; then
         dnl fail since mysql support was specifically enabled
         AC_MSG_ERROR(mysql_config not found in PATH)
      else
         dnl just warn and disable building
         AC_MSG_WARN(mysql_config not found in PATH)
         BUILD_MYSQL=no
      fi
   fi
fi
if test "x$BUILD_MYSQL" = "xyes"; then
   AC_DEFINE([HAVE_MYSQL], [1], ["mysqlclient support enabled"])
fi
AC_SUBST(BUILD_MYSQL)

dnl ----------------- Sphinx Search -----------------

AC_ARG_ENABLE([sphinx],
   AS_HELP_STRING([--disable-sphinx], [disable sphinx search [no]]),
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

dnl ----------------- V8 -----------------

AC_ARG_ENABLE([v8],
   AS_HELP_STRING([--enable-v8], [enable v8 [no]]),
   [
      case "${enableval}" in
         yes) BUILD_V8=yes ;;
         no) BUILD_V8=no ;;
         *) AC_MSG_ERROR(bad value ${enableval} for --enable-v8) ;;
      esac
   ], [BUILD_V8=no])  dnl Default value
# Check for v8 support
if test "x$BUILD_V8" = "xyes"; then
   AC_CHECK_HEADERS([v8.h],
      [],
      [AC_MSG_ERROR([v8 support requested but v8.h not found])])
fi
AC_SUBST(BUILD_V8)

dnl ----------------- DynamicObject statistics -----------------

dnl low level DynamicObject statistics

AC_ARG_ENABLE([dyno-counts],
   AS_HELP_STRING([--enable-dyno-counts],
      [enable DynamicObject type counts [no]]),
   [
      case "${enableval}" in
         yes) AC_DEFINE([MO_DYNO_COUNTS], [1],
	    ["Record internal DynamicObject statistics"])
	    MO_DYNO_STATS_MSG="counts $MO_DYNO_STATS_MSG"
	    ;;
         no) ;;
         *) AC_MSG_ERROR(bad value ${enableval} for --enable-dyno-counts) ;;
      esac
   ], [])  dnl Default value

AC_ARG_ENABLE([dyno-key-counts],
   AS_HELP_STRING([--enable-dyno-key-counts],
      [enable DynamicObject Map key counts [no]]),
   [
      case "${enableval}" in
         yes) AC_DEFINE([MO_DYNO_KEY_COUNTS], [1],
	    ["Record internal DynamicObject Map key statistics"])
	    MO_DYNO_STATS_MSG="key-counts $MO_DYNO_STATS_MSG"
	    ;;
         no) ;;
         *) AC_MSG_ERROR(bad value ${enableval} for --enable-dyno-key-counts) ;;
      esac
   ], [])  dnl Default value

dnl ----------------- substitutions -----------------

# Make all the proper substitutions
AC_SUBST(MONARCH_DIR)

AC_SUBST(MONARCH_VERSION_MAJOR)
AC_SUBST(MONARCH_VERSION_MINOR)
AC_SUBST(MONARCH_VERSION_MICRO)
AC_SUBST(DATE_RFC_2822)
AC_SUBST(FULL_DOTTED_VERSION)
AC_SUBST(FULL_VERSION)

AC_SUBST(PYTHON)

# Generating files
AC_CONFIG_FILES([
   .gitignore
   Makefile
   setup/Makefile.base
   setup/docs.doxygen
   configs/apps/js.config
   configs/apps/pong.config
   configs/apps/test.config
   cpp/3rdparty/Makefile
   cpp/app/Makefile
   cpp/apps/js/Makefile
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
   cpp/v8/Makefile
   cpp/validation/Makefile
   cpp/ws/Makefile
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

# Setup extra build configuration

MO_MSG_CONFIG_BEGIN([Monarch Options])
MO_MSG_CONFIG_OPTION([Dyno Stats], [$MO_DYNO_STATS_MSG])
MO_MSG_CONFIG_END

MO_MSG_CONFIG_BEGIN([Features])
MO_MSG_CONFIG_OPTION_IF([librdfa],
   [test "x$HAVE_LIBRDFA" = xyes],
   [enabled], [disabled])
MO_MSG_CONFIG_OPTION_IF([MySQL],
   [test "x$BUILD_MYSQL" = xyes],
   [enabled], [disabled (use --enable-mysql to enable)])
MO_MSG_CONFIG_OPTION_IF([Sphinx],
   [test "x$BUILD_SPHINX" = xyes],
   [enabled], [disabled (use --enable-sphinx to enable)])
MO_MSG_CONFIG_OPTION_IF([V8],
   [test "x$BUILD_V8" = xyes],
   [enabled], [disabled (use --enable-v8 to enable)])
MO_MSG_CONFIG_END

# Output build configuration
MO_MSG_CONFIG_OUTPUT([Monarch Build Configuration])
