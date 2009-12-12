/*
 * Copyright (c) 2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_rt_Platform_H
#define monarch_rt_Platform_H

#include <inttypes.h>

#include "monarch/rt/DynamicObject.h"

namespace monarch
{
namespace rt
{

/**
 * The Platform class provides details on the current platform and its specific
 * features and capabilities.
 *
 * @author David I. Lehn <dlehn@digitalbazaar.com>
 */
class Platform
{
public:
   /**
    * Information on a platform.
    *
    * PlatformInfo
    * {
    *    "spec": String, (FIXME: not implemented yet)
    *    "os": String,
    *    "cpuType": Int32,
    *    "cpuSubType": Int32, (FIXME: not implemented yet)
    *    "capabilities": {key=value} (FIXME: not implemented yet)
    * }
    *
    * @member spec combined architecture specficiation string
    * @member os operating system
    * @member cpuType primary CPU architecture type
    * @member cpuSubType secondary CPU architecture type
    * @member capabilities OS, CPU, and hardware dependent features
    */
   typedef monarch::rt::DynamicObject PlatformInfo;

public:
   /**
    * Initialize static Platform data. If getCurrent() will be called this
    * method must be called first and only once.
    *
    * @return true on succes, false if exception occured.
    */
   static bool initialize();

   /**
    * Cleanup static Platform data. If initialize() was called, then cleanup
    * should be called only once.
    */
   static void cleanup();

   /**
    * Gets the dynamic library prefix for a specific operating system.
    * For example, no prefix ("") for Windows, "lib" for Mac OS X and Linux.
    *
    * @return the dynamic library extension or NULL if not known.
    */
   static const char* getDynamicLibraryPrefix(const char* os);

   /**
    * Gets the dynamic library extension for a specific operating system.
    *
    * @return the dynamic library extension or NULL if not known.
    */
   static const char* getDynamicLibraryExt(const char* os);

   /**
    * Gets information for the current platform. The returned PlatformInfo
    * is read-only.
    *
    * @return the current platform info.
    */
   static PlatformInfo& getCurrent();
};

} // end namespace rt
} // end namespace monarch
#endif
