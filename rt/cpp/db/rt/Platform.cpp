/*
 * Copyright (c) 2009 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/rt/Platform.h"

#include <stddef.h>

using namespace db::rt;

// Note: do not initialize these, they are automatically
// initialized to NULL before main -- and sometimes if dlopen
// is called, it will try to re-init these, stomping on
// the existing values, leaking memory, and generally stuffing
// everything up
static DynamicObject* sDynLibExtMap;
static Platform::PlatformInfo* sCurrent;

bool Platform::initialize()
{
   if(sDynLibExtMap == NULL)
   {
      sDynLibExtMap = new DynamicObject();
      
      // dynamic library extension map
      (*sDynLibExtMap)["linux"] = "so";
      (*sDynLibExtMap)["windows"] = "dll";
      (*sDynLibExtMap)["macos"] = "dylib";
   }
   
   if(sCurrent == NULL)
   {
      sCurrent = new PlatformInfo();
      
      // Operating System
#if defined(LINUX)
      (*sCurrent)["os"] = "linux";
#elif defined(MACOS)
      (*sCurrent)["os"] = "macos";
#elif defined(WIN32)
      (*sCurrent)["os"] = "windows";
#else
#error Platform: Unknown OS.
#endif
      
      // Primary CPU type
#if defined(__i386__)
      (*sCurrent)["cpuType"] = "x86";
#elif defined(__x86_64__)
      (*sCurrent)["cpuType"] = "x86_64";
#elif defined(__ppc__)
      (*sCurrent)["cpuType"] = "ppc";
//#elif defined(__ppc64__)
//      (*sCurrent)["cpuType"] = "ppc64";
//#elif defined(__arm__)
//      (*sCurrent)["cpuType"] = "arm";
#else
#error Platform: Unknown CPU type.
#endif
      
      // Secondary CPU type
      // FIXME: add specific CPU type detection
      (*sCurrent)["cpuSubType"] = "";
   }
   
   return true;
}

void Platform::cleanup()
{
   if(sDynLibExtMap != NULL)
   {
      delete sDynLibExtMap;
      sDynLibExtMap = NULL;
   }
   if(sCurrent != NULL)
   {
      delete sCurrent;
      sCurrent = NULL;
   }
}

const char* Platform::getDynamicLibraryExt(const char* os)
{
   const char* rval = NULL;
   if(os != NULL && (*sDynLibExtMap)->hasMember(os))
   {
      rval = (*sDynLibExtMap)[os]->getString();
   }
   return rval;
}

Platform::PlatformInfo& Platform::getCurrent()
{
   return *sCurrent;
}
