/*
 * Copyright (c) 2010 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_v8_V8ModuleApi_h
#define monarch_v8_V8ModuleApi_h

#include "monarch/kernel/MicroKernelModuleApi.h"
#include "monarch/v8/V8EngineApi.h"

namespace monarch
{
namespace v8
{

/**
 * A V8ModuleApi provides the application programming interface for the
 * V8Module.
 *
 * @author David I. Lehn
 */
class V8ModuleApi : public monarch::kernel::MicroKernelModuleApi
{
public:
   /**
    * Creates a new V8ModuleApi.
    */
   V8ModuleApi() {};

   /**
    * Destructs this V8ModuleApi.
    */
   virtual ~V8ModuleApi() {};

   /**
    * Create a V8Engine object. Call initialize(...) on the returned
    * object to perform futher setup.
    *
    * @param v8Engine new V8Engine object to set.
    *
    * @return true on success, false and exception set on failure.
    */
   virtual bool createEngine(V8EngineRef& v8Engine) = 0;
};

} // end namespace v8
} // end namespace monarch
#endif
