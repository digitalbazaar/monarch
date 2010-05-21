/*
 * Copyright (c) 2010 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_app_AppFactory_h
#define monarch_app_AppFactory_h

#include "monarch/app/App.h"
#include "monarch/kernel/MicroKernelModule.h"
#include "monarch/modest/Module.h"

namespace monarch
{
namespace app
{

/**
 * An AppFactory is used by the AppRunner's Kernel to create an App. The
 * kernel will look a module of type "monarch.app.AppFactory" and use its
 * createApp() call to create an App to run in the AppRunner.
 *
 * This class provides a base module that can be used for subclasses.
 *
 * @author David I. Lehn
 * @author Dave Longley
 */
class AppFactory :
   public monarch::kernel::MicroKernelModule,
   public monarch::kernel::MicroKernelModuleApi
{
protected:
   /**
    * Info for this module.
    */
   monarch::rt::DynamicObject mInfo;

   /**
    * MicroKernel used for this App.
    */
   monarch::kernel::MicroKernel* mMicroKernel;

public:
   /**
    * Create an AppFactory instance.
    *
    * @param name the name for this MicroKernelModule.
    * @param version the version for this MicroKernelModule (major.minor).
    */
   AppFactory(const char* name, const char* version);

   /**
    * Deconstruct this AppFactory instance.
    */
   virtual ~AppFactory();

   /**
    * Add a dependency.
    *
    * @param name the module name of the dependency.
    * @param version the module version of the dependency.
    */
   virtual void addDependency(const char* name, const char* version);

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

   /**
    * Creates an App object.
    *
    * @return the created App object or NULL if an exception occurred.
    */
   virtual App* createApp() = 0;

   /**
    * Destroys an App object.
    *
    * @param app the App to destroy.
    */
   virtual void destroyApp(App* app);
};

/**
 * Free an AppFactory. Common case that just deletes the module.
 */
void freeAppFactory(monarch::modest::Module* m);

} // end namespace app
} // end namespace monarch

#endif
