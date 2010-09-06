/*
 * Copyright (c) 2010 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/v8/V8Module.h"
#include "monarch/v8/V8Controller.h"

using namespace monarch::kernel;
using namespace monarch::logging;
using namespace monarch::modest;
using namespace monarch::rt;
using namespace monarch::v8;

// Logging category initialized during module initialization.
Category* MO_V8_CAT;

V8Module::V8Module() :
   MicroKernelModule("monarch.v8.V8", "1.0"),
   mApi(NULL)
{
}

V8Module::~V8Module()
{
}

DynamicObject V8Module::getDependencyInfo()
{
   DynamicObject rval;

   // set name, version, and type for this module
   rval["name"] = mId.name;
   rval["version"] = mId.version;
   rval["type"] = "monarch.v8";

   // no dependencies
   rval["dependencies"]->setType(Array);

   return rval;
}

bool V8Module::initialize(MicroKernel* k)
{
   bool rval = true;

   MO_V8_CAT = new Category(
      "MO_V8",
      "Monarch V8 Engine",
      NULL);

   mApi = new V8Controller();
   if(!(rval = mApi->initialize(k)))
   {
      delete mApi;
      mApi = NULL;
   }

   return rval;
}

void V8Module::cleanup(MicroKernel* k)
{
   if(mApi != NULL)
   {
      mApi->cleanup();
      delete mApi;
      mApi = NULL;
   }

   delete MO_V8_CAT;
   MO_V8_CAT = NULL;
}

MicroKernelModuleApi* V8Module::getApi(MicroKernel* k)
{
   return mApi;
}

Module* createModestModule()
{
   return new V8Module();
}

void freeModestModule(Module* m)
{
   delete m;
}
