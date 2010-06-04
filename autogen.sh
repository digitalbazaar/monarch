#!/bin/sh

# Check and add potential aclocal dirs
MAYBE_AC_DIRS="
   /usr/local/share/aclocal
   /opt/local/share/aclocal
   /sw/share/aclocal
   "
ACDIRS=
for dir in $MAYBE_AC_DIRS; do
   if test -d $dir; then
      ACDIRS="$ACDIRS -I $dir"
   fi
done
aclocal $ACDIRS
autoconf
