# Configure script for Digital Bazaar common source distribution
# Usage: Run ./configure once 
# Author: Manu Sporny

AC_INIT(dbcore, 3.0, support@digitalbazaar.com)

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
echo "C++ Compiler   : $CXX"

# Generating files
AC_OUTPUT(Makefile \
          setup/Makefile.base
          rt/cpp/Makefile)
