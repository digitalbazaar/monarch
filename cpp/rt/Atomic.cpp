/*
 * Copyright (c) 2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "db/rt/Atomic.h"

// include NULL
#include <cstddef>

#ifdef WIN32
#include <windows.h>
#include <malloc.h>
#endif

using namespace db::rt;

void* Atomic::mallocAligned(size_t size)
{
#if 0//def WIN32
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
