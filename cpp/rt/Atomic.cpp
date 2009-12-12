/*
 * Copyright (c) 2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/rt/Atomic.h"

// include NULL
#include <cstddef>

#ifdef WIN32
#include <windows.h>
#include <malloc.h>
#endif

using namespace monarch::rt;

void* Atomic::mallocAligned(size_t size)
{
#if 0//def WIN32
   // FIXME: _aligned_malloc is defined by msvcrXX.dll ... which is giving
   // us headaches figuring out with mingw32 ... for now, since we build with
   // gcc, memory should already be aligned on the proper boundaries
   // automatically ... but should we do some strange packing or other funny
   // business like compiling using MSVC, we might want to revisit this

   // must align on a 32-bit boundary for windows
   return _aligned_malloc(size, ALIGN_BITS);
#else
   return malloc(size);
#endif
}

void Atomic::freeAligned(void* ptr)
{
   if(ptr != NULL)
   {
#if 0//def WIN32
      _aligned_free(ptr);
#else
      free(ptr);
#endif
   }
}
