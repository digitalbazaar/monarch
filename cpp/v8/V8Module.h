/*
 * Copyright (c) 2010 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_v8_V8Module_h
#define monarch_v8_V8Module_h

#include "monarch/kernel/MicroKernelModule.h"
#include "monarch/logging/Category.h"
#include "monarch/v8/V8ModuleApi.h"

// module logging category
extern monarch::logging::Category* MO_V8_CAT;

namespace monarch
{
namespace v8
{

class V8Controller;

/**
 * A V8Module is a Module that can run JavaScript using the V8 engine.
 * 
 * @author David I. Lehn
 */
class V8Module : public monarch::kernel::MicroKernelModule
{
protected:
   /**
    * The API implementation for this module.
    */
   V8Controller* mApi;

public:
   /**
    * Creates a new V8Module.
    */
   V8Module();
   
   /**
    * Destructs this V8Module.
    */
   virtual ~V8Module();
   
   /**
    * Gets dependency information.
    *
    * @return the dependency information.
    */
   virtual monarch::rt::DynamicObject getDependencyInfo();

   /**
    * Initializes this Module with the passed MicroKernel.
    *
    * @param k the MicroKernel.
    *
    * @return true if initialized, false if an Exception occurred.
    */
   virtual bool initialize(monarch::kernel::MicroKernel* k);

   /**
    * Cleans up this Module just prior to its unloading.
    *
    * @param k the MicroKernel.
    */
   virtual void cleanup(monarch::kernel::MicroKernel* k);

   /**
    * Gets the API for this MicroKernelModule.
    *
    * @param k the MicroKernel that loaded this module.
    *
    * @return the API for this MicroKernelModule.
    */
   virtual monarch::kernel::MicroKernelModuleApi* getApi(
      monarch::kernel::MicroKernel* k);
};

} // end namespace v8
} // end namespace monarch
#endif
