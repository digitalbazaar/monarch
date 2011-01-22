dnl Monarch utilities
dnl Copyright 2010 Digital Bazaar, Inc.

dnl MO_MSG_CONFIG_TEXT
dnl MO_MSG_CONFIG_BEGIN
dnl MO_MSG_CONFIG_END
dnl MO_MSG_CONFIG_OPTION
dnl MO_MSG_CONFIG_OUTPUT

dnl ----------------- Config Messages -----------------

dnl Add arbitrary text to the config message.
dnl
dnl MO_MSG_CONFIG_TEXT(TEXT)
dnl
AC_DEFUN([MO_MSG_CONFIG_TEXT],
[
   dnl MO_MSG_CONFIG_="${MO_MSG_CONFIG_}"[$1]"m4_newline()"
   m4_append([MO_MSG_CONFIG_],[$1])
   m4_append([MO_MSG_CONFIG_],[m4_newline()])
])

dnl Begin a config option section.
dnl
dnl MO_MSG_CONFIG_BEGIN(TITLE)
dnl
AC_DEFUN([MO_MSG_CONFIG_BEGIN],
[
   MO_MSG_CONFIG_="${MO_MSG_CONFIG_}----------------- [$1] -----------------m4_newline()"
   dnl MO_MSG_CONFIG_TEXT([----------------- $1 -----------------])
])

dnl End a config option section.
dnl
dnl MO_MSG_CONFIG_END
dnl
AC_DEFUN([MO_MSG_CONFIG_END],
[
   dnl MO_MSG_CONFIG_TEXT(m4_newline())
   MO_MSG_CONFIG_="${MO_MSG_CONFIG_}m4_newline()"
])

dnl Add a config option.
dnl
dnl MO_MSG_CONFIG_OPTION(NAME, VALUE)
dnl
AC_DEFUN([MO_MSG_CONFIG_OPTION],
[
   dnl MO_MSG_CONFIG_TEXT(m4_text_wrap([$2], [               ], [$1])m4_newline())
   MO_MSG_CONFIG_="${MO_MSG_CONFIG_}m4_text_wrap([$2], [                  ], [$1 ])m4_newline()"
])

dnl Add a config option using a test and optional output.
dnl
dnl MO_MSG_CONFIG_OPTION_IF(NAME, TEST, VALUE-IF-TRUE, VALUE-IF-FALSE)
dnl
AC_DEFUN([MO_MSG_CONFIG_OPTION_IF],
[
   AS_IF([$2],[_MO_MSG="$3"],[_MO_MSG="$4"])
   MO_MSG_CONFIG_OPTION([$1], [$_MO_MSG])
])

dnl Output config message with optional title.
dnl
dnl MO_MSG_CONFIG_OUTPUT([TITLE])
dnl
AC_DEFUN([MO_MSG_CONFIG_OUTPUT],
[
   AS_IF([test "x$1" != x],
   [
      AS_ECHO
      AS_ECHO(["================= [$1] =================]")
   ])
   AS_ECHO_N(["$MO_MSG_CONFIG_"])
])

dnl ----------------- File list algorithm -----------------
AC_DEFUN([MO_ALL_FILES_ALGORITHM],
[
ALL_FILES_ALGORITHM="\$(shell find $1 -type f -name \"config.status\" m4_foreach(rex,[m4_shift($@)], -o -name \"rex\" ))"
])

