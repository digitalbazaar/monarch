dnl Monarch platform macros
dnl Copyright 2010 Digital Bazaar, Inc.

dnl MO_PLATFORM

dnl ----------------- detect target platform -----------------

dnl Use $host to determine platform to target.
dnl Defaults to $build but can override with configure --host option.
dnl
dnl Sets the following with AC_SUBST:
dnl    PLATFORM (linux|macos|windows)
dnl    CPU_ARCHITECTURE (i686|athlon64)
dnl    PLATFORM_KNOWN (yes|no) (no if guessing on type)
dnl    BUILD_FOR_LINUX (yes|no)
dnl    BUILD_FOR_MACOS (yes|no)
dnl    BUILD_FOR_WINDOWS (yes|no)
AC_DEFUN([MO_PLATFORM],
[
   # Setup build and host configuration
   AC_REQUIRE([AC_CANONICAL_BUILD])
   AC_REQUIRE([AC_CANONICAL_HOST])

   AC_MSG_CHECKING([CPU architecture type])
   AS_CASE([$host],
      [x86_64-*-*], [CPU_ARCHITECTURE=athlon64],
      [CPU_ARCHITECTURE=i686])
   AC_MSG_RESULT([$CPU_ARCHITECTURE])

   AC_MSG_CHECKING([platform type])
   AS_CASE([$host],
      [*-*-linux*], [PLATFORM=linux],
      [*-*-darwin*|*-*-macos*], [PLATFORM=macos],
      [*-*-mingw32*|*-*-cygwin*|*-*-windows*|*-*-winnt*], [PLATFORM=windows],
      [PLATFORM=unknown])
   AC_MSG_RESULT([$PLATFORM])
   if test "x$PLATFORM" = xunknown; then
      AC_MSG_WARN([Unknown platform: will build as if linux])
      PLATFORM=linux
      PLATFORM_KNOWN=no
   else
      PLATFORM_KNOWN=yes
   fi

   dnl Default all platforms off
   BUILD_FOR_LINUX=no
   BUILD_FOR_MACOS=no
   BUILD_FOR_WINDOWS=no
   dnl Set proper BUILD_FOR_* var
   AS_CASE([$PLATFORM],
      [linux], [BUILD_FOR_LINUX=yes],
      [macos], [BUILD_FOR_MACOS=yes],
      [windows], [BUILD_FOR_WINDOWS=yes])

   dnl Setup substitutions
   AC_SUBST(PLATFORM)
   AC_SUBST(CPU_ARCHITECTURE)
   AC_SUBST(BUILD_FOR_LINUX)
   AC_SUBST(BUILD_FOR_WINDOWS)
   AC_SUBST(BUILD_FOR_MACOS)

   dnl Config output
   MO_MSG_CONFIG_BEGIN([Platform])
   MO_MSG_CONFIG_OPTION([Build system], [$build])
   MO_MSG_CONFIG_OPTION([Target system], [$host (use --host to change)])
   MO_MSG_CONFIG_OPTION_IF([Target platform],
      [test "x$PLATFORM_KNOWN" = xyes],
      [$PLATFORM], [$PLATFORM (detection failed, using default)])
   MO_MSG_CONFIG_END
])
