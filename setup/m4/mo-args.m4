dnl Monarch misc options
dnl Copyright 2010 Digital Bazaar, Inc.

dnl MO_ARG_DEBUG
dnl MO_ARG_LOG_LINE_NUMBERS
dnl MO_ARG_OPT
dnl MO_ARG_TESTS
dnl MO_ARG_DOCS

dnl ----------------- control debugging  -----------------

AC_DEFUN([MO_ARG_DEBUG],
[
   dnl compile with -Werror
   AC_ARG_ENABLE([werror],
      AC_HELP_STRING(
         [--enable-werror],
         [compile with -Werror [no]]),
      [
      case "${enableval}" in
         yes) _MO_CXX_DEBUG_FLAGS="-Werror" ;;
         no)  ;;
         *)   AC_MSG_ERROR(bad value ${enableval} for --enable-werror) ;;
      esac
      ],
      []) dnl Default value

   dnl combine flags
   MO_CXX_DEBUG_FLAGS="$_MO_CXX_DEBUG_FLAGS $MO_CXX_DEBUG_FLAGS"

   dnl check flags
   AS_CXX_COMPILER_FLAG([$MO_CXX_DEBUG_FLAGS], [flags_ok=yes], [flags_ok=no])

   if test "x$flags_ok" != xyes; then
      AC_MSG_ERROR(Invalid debug flags: "$MO_CXX_DEBUG_FLAGS")
   fi

   CXXFLAGS="$CXXFLAGS $MO_CXX_DEBUG_FLAGS"
   MO_MSG_CONFIG_OPTION([Debug], [$MO_CXX_DEBUG_FLAGS])
])

dnl ----------------- control log line numbers -----------------

AC_DEFUN([MO_ARG_LOG_LINE_NUMBERS],
[
   AC_ARG_ENABLE([log-line-numbers],
      AC_HELP_STRING(
         [--disable-log-line-numbers],
         [disable log line numbers [no]]),
      [
      case "${enableval}" in
         yes) MO_DISABLE_LOG_LINE_NUMBERS=no ;;
         no)  MO_DISABLE_LOG_LINE_NUMBERS=yes ;;
         *)   AC_MSG_ERROR(bad value ${enableval} for --enable-log-line-numbers) ;;
      esac
      ],
      [MO_DISABLE_LOG_LINE_NUMBERS=no]) dnl Default value
   if test "x$MO_DISABLE_LOG_LINE_NUMBERS" = xyes; then
      AC_DEFINE([MO_DISABLE_LOG_LINE_NUMBERS], [1],
         [Enable log line numbers.])
   fi

   MO_MSG_CONFIG_OPTION_IF([Log line numbers],
      [test "x$MO_DISABLE_LOG_LINE_NUMBERS" = xno],
      [enabled], [disabled (use --enable-log-line-numbers to enable)])
])

dnl ----------------- control optimizations -----------------

AC_DEFUN([MO_ARG_OPT],
[
   dnl FIXME: compiler specific flags
   dnl default flags
   _MO_OPT_LEVEL_FLAGS="-O"
   _MO_OPT_EXTRA_FLAGS="-fno-exceptions"
  
   dnl "zero" level optimization
   AC_ARG_ENABLE([optimization],
      AC_HELP_STRING(
         [--disable-optimization],
         [disable normal compiler optimizations [no]]),
      [
      case "${enableval}" in
         yes)  ;;
         no)  _MO_OPT_LEVEL_FLAGS="-O0" ;;
         *)   AC_MSG_ERROR(bad value ${enableval} for --enable-optimization) ;;
      esac
      ],
      []) dnl Default value

   dnl "zero" level optimization and disable extra flags
   AC_ARG_ENABLE([all-optimization],
      AC_HELP_STRING(
         [--disable-all-optimization],
         [disable all compiler optimizations [no]]),
      [
      case "${enableval}" in
         yes)  ;;
         no)  _MO_OPT_LEVEL_FLAGS="-O0"
              _MO_OPT_EXTRA_FLAGS="" ;;
         *)   AC_MSG_ERROR(bad value ${enableval} for --enable-all-optimization) ;;
      esac
      ],
      []) dnl Default value

   dnl combine flags
   MO_CXX_OPT_FLAGS="$_MO_OPT_LEVEL_FLAGS $_MO_OPT_EXTRA_FLAGS $MO_CXX_OPT_FLAGS"

   dnl option to fully specify optimization flags
   AC_ARG_WITH(opt-flags,
      AC_HELP_STRING([--with-opt-flags=FLAGS],
         [use FLAGS for optimization]),
      [MO_CXX_OPT_FLAGS="${withval}"],
      []) dnl Default value

   dnl check flags
   AS_CXX_COMPILER_FLAG([$MO_CXX_OPT_FLAGS], [flags_ok=yes], [flags_ok=no])

   if test "x$flags_ok" != xyes; then
      AC_MSG_ERROR(Invalid optimization flags: "$MO_CXX_OPT_FLAGS")
   fi

   CXXFLAGS="$CXXFLAGS $MO_CXX_OPT_FLAGS"
   MO_MSG_CONFIG_OPTION([Optimization], [$MO_CXX_OPT_FLAGS])
])

dnl ----------------- control building and running tests -----------------

AC_DEFUN([MO_ARG_TESTS],
[
   AC_ARG_ENABLE([tests],
      AC_HELP_STRING([--disable-tests], [disable building test apps [no]]),
      [
         case "${enableval}" in
            yes) BUILD_TESTS=yes;
	         BUILD_TESTS_SET=yes ;;
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

   MO_MSG_CONFIG_OPTION_IF([Tests],
      [test "x$BUILD_TESTS" = xyes],
      [enabled], [disabled (use --enable-tests to enable)])
])

dnl ----------------- control building docs -----------------

AC_DEFUN([MO_ARG_DOCS],
[
   AC_ARG_ENABLE([docs],
      AC_HELP_STRING([--enable-docs], [enable building docs [no]]),
      [
         case "${enableval}" in
            yes) BUILD_DOCS=yes ;;
            no)  BUILD_DOCS=no ;;
            *)   AC_MSG_ERROR(bad value ${enableval} for --enable-docs) ;;
         esac
      ],
      [BUILD_DOCS=no]) dnl Default value
   AC_SUBST(BUILD_DOCS)

   MO_MSG_CONFIG_OPTION_IF([Docs],
      [test "x$BUILD_DOCS" = xyes],
      [enabled], [disabled (use --enable-docs to enable)])
])
