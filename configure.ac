# Configure script for Digital Bazaar common source distribution
# Usage: Run ./configure once 
# Author: Manu Sporny

AC_INIT(dbcore, 3.0, support@digitalbazaar.com)
AC_CONFIG_AUX_DIR(setup)
AC_PROG_INSTALL
AC_CANONICAL_BUILD

# Setup standard build environment variables
DBCOREDIR=`pwd`

echo -e "\n--------- Configuring DB Core Build Environment -----------"

# Checking for standard build tools
#AC_PROG_CXX
#AC_LANG(C++)

# Perform compilation environment tests
#AC_CHECK_HEADERS(iostream)

# Make sure the proper libraries exist
#AC_CHECK_LIB(pthread, main, [], AC_MSG_ERROR(could not find pthread library))
#AC_CHECK_LIB(crypto, main, [], AC_MSG_ERROR(could not find crypto library))
#AC_CHECK_LIB(ssl, main, [], AC_MSG_ERROR(could not find ssl library))

# Make all the proper substitutions
AC_SUBST(DBCOREDIR)
AC_SUBST(JAVADOC)

echo -e "\n--------- DB Core Build Environment -----------"
echo "System      : $PACKAGE_STRING"
echo "Build system: $build_cpu $build_vendor $build_os"
echo "C++ Compiler: $CXX"

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
