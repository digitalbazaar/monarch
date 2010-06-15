dnl Common Monarch configure-time options

dnl MO_ARG_PLATFORM
dnl MO_ARG_DEBUG
dnl MO_ARG_LOG_LINE_NUMBERS
dnl MO_ARG_OPT
dnl MO_ARG_TESTS

dnl ----------------- choose target platform -----------------

AC_DEFUN([MO_ARG_PLATFORM],
[
   dnl platform choice
   AC_ARG_ENABLE([linux],
      AS_HELP_STRING([--enable-linux], [compile for linux [yes]]),
      [
         case "${enableval}" in
            yes) BUILD_FOR_LINUX=yes 
                 BUILD_FOR_MACOS=no
                 BUILD_FOR_WINDOWS=no
                 ;;
            no) BUILD_FOR_LINUX=no ;;
            *) AC_MSG_ERROR(bad value ${enableval} for --enable-linux) ;;
         esac
      ], [BUILD_FOR_LINUX=yes]) dnl Default value

   AC_ARG_ENABLE([windows],
      AS_HELP_STRING([--enable-windows], [compile for windows [no]]),
      [
         case "${enableval}" in
            yes) BUILD_FOR_LINUX=no
                 BUILD_FOR_MACOS=no
                 BUILD_FOR_WINDOWS=yes 
                 ;;
            no) BUILD_FOR_WINDOWS=no ;;
            *) AC_MSG_ERROR(bad value ${enableval} for --enable-windows) ;;
         esac
      ], [BUILD_FOR_WINDOWS=no]) dnl Default value

   AC_ARG_ENABLE([macos],
      AS_HELP_STRING([--enable-macos], [compile for MacOS [no]]),
      [
         case "${enableval}" in
            yes) BUILD_FOR_LINUX=no
                 BUILD_FOR_MACOS=yes 
                 BUILD_FOR_WINDOWS=no
                 ;;
            no) BUILD_FOR_MACOS=no ;;
            *) AC_MSG_ERROR(bad value ${enableval} for --enable-macos) ;;
         esac
      ], [BUILD_FOR_MACOS=no]) dnl Default value
])

dnl ----------------- control debugging  -----------------

AC_DEFUN([MO_ARG_DEBUG],
[
   CXXFLAGS="$CXXFLAGS $MO_CXX_DEBUG_FLAGS"
])

dnl ----------------- disable log line numbers -----------------

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
      AC_DEFINE(MO_DISABLE_LOG_LINE_NUMBERS, 1,
         [Enable log line numbers.])
   fi
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
   MO_CXX_OPT_FLAGS="$_MO_OPT_LEVEL_FLAGS $_MO_OPT_EXTRA_FLAGS"

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
])

dnl ----------------- disable test building and running -----------------

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
])
