/*
 * Copyright (c) 2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "db/rt/Platform.h"

#include <stddef.h>

using namespace db::rt;

// Note: do not initialize these, they are automatically
// initialized to NULL before main -- and sometimes if dlopen
// is called, it will try to re-init these, stomping on
// the existing values, leaking memory, and generally stuffing
// everything up
static DynamicObject* sCommonInfo;
static Platform::PlatformInfo* sCurrentInfo;

/*
 * sCommonInfo:
 * {
 *    "os": {osName: osInfo}
 * }
 * osInfo:
 * {
 *    "dynamicLibPrefix": String
 *    "dynamicLibExt": String
 * }
 * @member dynamicLibPrefix prefix for dynamic libraries
 * @member dynamicLibExt extension for dynamic libraries
 */

bool Platform::initialize()
{
   if(sCommonInfo == NULL)
   {
      sCommonInfo = new DynamicObject();

      // dynamic library prefix map
      (*sCommonInfo)["os"]["linux"]["dynamicLibPrefix"] = "lib";
      (*sCommonInfo)["os"]["windows"]["dynamicLibPrefix"] = "";
      (*sCommonInfo)["os"]["macos"]["dynamicLibPrefix"] = "lib";

      // dynamic library extension map
      (*sCommonInfo)["os"]["linux"]["dynamicLibExt"] = "so";
      (*sCommonInfo)["os"]["windows"]["dynamicLibExt"] = "dll";
      (*sCommonInfo)["os"]["macos"]["dynamicLibExt"] = "dylib";
   }

   if(sCurrentInfo == NULL)
   {
      sCurrentInfo = new PlatformInfo();

      // Operating System
#if defined(LINUX)
      (*sCurrentInfo)["os"] = "linux";
#elif defined(MACOS)
      (*sCurrentInfo)["os"] = "macos";
#elif defined(WIN32)
      (*sCurrentInfo)["os"] = "windows";
#else
#error Platform: Unknown OS.
#endif

      // Primary CPU type
#if defined(__i386__)
      (*sCurrentInfo)["cpuType"] = "x86";
#elif defined(__x86_64__)
      (*sCurrentInfo)["cpuType"] = "x86_64";
#elif defined(__ppc__)
      (*sCurrentInfo)["cpuType"] = "ppc";
//#elif defined(__ppc64__)
//      (*sCurrentInfo)["cpuType"] = "ppc64";
//#elif defined(__arm__)
//      (*sCurrentInfo)["cpuType"] = "arm";
#else
#error Platform: Unknown CPU type.
#endif

      // Secondary CPU type
      // FIXME: add specific CPU type detection
      (*sCurrentInfo)["cpuSubType"] = "";
   }

   return true;
}

void Platform::cleanup()
{
   if(sCommonInfo != NULL)
   {
      delete sCommonInfo;
      sCommonInfo = NULL;
   }
   if(sCurrentInfo != NULL)
   {
      delete sCurrentInfo;
      sCurrentInfo = NULL;
   }
}

const char* Platform::getDynamicLibraryPrefix(const char* os)
{
   const char* rval = NULL;
   if(os != NULL && (*sCommonInfo)["os"]->hasMember(os))
   {
      rval = (*sCommonInfo)["os"][os]["dynamicLibPrefix"]->getString();
   }
   return rval;
}

const char* Platform::getDynamicLibraryExt(const char* os)
{
   const char* rval = NULL;
   if(os != NULL && (*sCommonInfo)["os"]->hasMember(os))
   {
      rval = (*sCommonInfo)["os"][os]["dynamicLibExt"]->getString();
   }
   return rval;
}

Platform::PlatformInfo& Platform::getCurrent()
{
   return *sCurrentInfo;
}
