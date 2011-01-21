dnl Monarch compiler macros
dnl Copyright 2010 Digital Bazaar, Inc.

dnl MO_COMPILER

dnl ----------------- compiler detection -----------------

dnl Use detected platform and autotools macros to detect compiler tools.
dnl Use standard env vars to override.  See configure --help.
dnl
dnl The following are set with AC_SUBST:
dnl    CC, CXX, AR, AS, STRIP, STRIP_FLAGS
dnl    LIB_PREFIX
dnl    DYNAMIC_LIB_EXT
dnl    STATIC_LIB_EXT
dnl    EXECUTABLE_EXT
dnl    OBJECT_EXT
AC_DEFUN([MO_COMPILER],
[
   AC_REQUIRE([MO_PLATFORM])

   MO_MSG_CONFIG_BEGIN([Compilation Tools])

   # Setup where to check for tools.
   #
   # Can override these prog lists with env vars.
   #
   # Using custom lists since the macro expansion of AC_PROG_CC/CXX will
   # cause only the first visible macro to function properly. Inside of
   # platform conditionals the other macros will produce incorrect
   # results.

   # Clang support
   AC_ARG_ENABLE([clang],
      AC_HELP_STRING([--enable-clang], [enable checking for clang [no]]),
      [
         case "${enableval}" in
            yes) CHECK_PROGS_CC_CLANG="clang";
                 CHECK_PROGS_CXX_CLANG="clang++" ;;
            no)  ;;
            *)   AC_MSG_ERROR(bad value ${enableval} for --enable-clang) ;;
         esac
      ],
      [:]) dnl Default value

   # Setup platform specific special checks.
   AS_CASE([$PLATFORM],
      [linux], [],
      [macos], [
         CHECK_PROGS_CC_PLATFORM="gcc-4"
         CHECK_PROGS_CXX_PLATFORM="g++-4"
      ],
      [windows], [])

   # If host_alias explictly set, check for host name based tools.
   # Special $host_cpu-$host_os is used due to special host aliases used
   # in Monarch that don't map to normal tool names when cross compiling
   # (linux, macos, windows).
   AS_IF([test "x$host_alias" != x],[
      CHECK_PROGS_CC_HOST="$host_alias-gcc $host_alias-cc $host_cpu-$host_os-gcc $host_cpu-$host_os-cc"
      CHECK_PROGS_CXX_HOST="$host_alias-g++ $host_alias-c++ $host_cpu-$host_os-g++ $host_cpu-$host_os-c++"
      CHECK_PROGS_AR_HOST="$host_alias-ar $host_cpu-$host_os-ar ar"
      CHECK_PROGS_AS_HOST="$host_alias-as $host_cpu-$host_os-as as"
      CHECK_PROGS_STRIP_HOST="$host_alias-strip $host_cpu-$host_os-strip"
   ])

   # Setup standard order of compile checks. Start with env vars.
   CHECK_PROGS_CC="\
      $CC \
      $CHECK_PROGS_CC_CLANG \
      $CHECK_PROGS_CC_PLATFORM \
      $CHECK_PROGS_CC_HOST \
      gcc cc"
   CHECK_PROGS_CXX="\
      $CXX \
      $CHECK_PROGS_CXX_CLANG \
      $CHECK_PROGS_CXX_PLATFORM \
      $CHECK_PROGS_CXX_HOST \
      g++ c++"
   CHECK_PROGS_AR="\
      $AR \
      $CHECK_PROGS_AR_PLATFORM \
      $CHECK_PROGS_AR_HOST \
      ar"
   CHECK_PROGS_AS="\
      $AS \
      $CHECK_PROGS_AS_PLATFORM \
      $CHECK_PROGS_AS_HOST \
      as"
   CHECK_PROGS_STRIP="\
      $STRIP \
      $CHECK_PROGS_STRIP_PLATFORM \
      $CHECK_PROGS_STRIP_HOST \
      strip"

   # Check for tools.
   # Doing a manual check for CC/CXX in order to do some custom compiler checks
   # before defaulting to AC_PROG_CC/CXX.  Still need to run
   # AC_PROG_CC/CXX so they can do addtional internal checks.
   AC_CHECK_PROGS(CC, [$CHECK_PROGS_CC])
   AC_PROG_CC
   if test "x$CC" = "x"; then
      AC_MSG_ERROR([C compiler not found])
   fi
   MO_MSG_CONFIG_OPTION([C Compiler], [$CC])

   AC_CHECK_PROGS(CXX, [$CHECK_PROGS_CXX])
   AC_PROG_CXX
   if test "x$CXX" = "x"; then
      AC_MSG_ERROR([C++ compiler not found])
   fi
   MO_MSG_CONFIG_OPTION([C++ Compiler], [$CXX])

   AC_CHECK_PROGS(AR, [$CHECK_PROGS_AR])
   if test "x$AR" = "x"; then
      AC_MSG_ERROR([Archiver not found])
   fi
   MO_MSG_CONFIG_OPTION([Archiver], [$AR])

   AC_CHECK_PROGS(AS, [$CHECK_PROGS_AS])
   if test "x$AS" = "x"; then
      AC_MSG_ERROR([Assembler not found])
   fi
   MO_MSG_CONFIG_OPTION([Assembler], [$AS])

   AC_CHECK_PROGS(STRIP, [$CHECK_PROGS_STRIP])
   if test "x$STRIP" = "x"; then
      AC_MSG_ERROR([Strip not found])
   fi
   MO_MSG_CONFIG_OPTION([Strip], [$STRIP])

   dnl Set various platform specific vars
   AS_CASE([$PLATFORM],
      [linux], [
         LIB_PREFIX=lib
         DYNAMIC_LIB_EXT=so
         STATIC_LIB_EXT=a
         STRIP_FLAGS="--strip-debug"
      ],
      [macos], [
         LDFLAGS="$LDFLAGS -L/sw/lib"
         LIB_PREFIX=lib
         DYNAMIC_LIB_EXT=dylib
         STATIC_LIB_EXT=a
         if test -d /sw/include; then
            CPPFLAGS="$CPPFLAGS -I/sw/include"
         fi
         STRIP_FLAGS="-S"
      ],
      [windows], [
         LDFLAGS="$LDFLAGS -Wl,--enable-auto-import"
         LIB_PREFIX=
         DYNAMIC_LIB_EXT=dll
         STATIC_LIB_EXT=a
         STRIP_FLAGS="--strip-debug"
      ])
   dnl Executable and object extension as determined by AC_PROG_CC/CXX.
   EXECUTABLE_EXT=$EXEEXT
   OBJECT_EXT=$OBJEXT

   dnl Setup substitutions
   dnl Prefixes and extentions
   AC_SUBST(LIB_PREFIX)
   AC_SUBST(DYNAMIC_LIB_EXT)
   AC_SUBST(STATIC_LIB_EXT)
   AC_SUBST(EXECUTABLE_EXT)
   AC_SUBST(OBJECT_EXT)
   dnl Library symbol stripping variables
   AC_SUBST(STRIP_FLAGS)

   MO_MSG_CONFIG_END
])
