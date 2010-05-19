/*
 * Copyright (c) 2010 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_app_AppPluginFactory_h
#define monarch_app_AppPluginFactory_h

#include "monarch/app/AppPlugin.h"
#include "monarch/kernel/MicroKernelModule.h"
#include "monarch/modest/Module.h"

namespace monarch
{
namespace app
{

/**
 * AppPluginFactories are used by the KernelPlugin to create AppPlugins. The
 * kernel will look for all modules of type "monarch.app.AppPluginFactory" and
 * use their createAppPlugin() call to create an AppPlugin to add to the App.
 *
 * This class provides a base plugin that can be used for subclasses.
 *
 * @author David I. Lehn
 */

class AppPluginFactory :
   public monarch::kernel::MicroKernelModule,
   public monarch::kernel::MicroKernelModuleApi
{
protected:
   /**
    * Info for this plugin.
    */
   monarch::rt::DynamicObject mInfo;

   /**
    * MicroKernel used for this App.
    */
   monarch::kernel::MicroKernel* mMicroKernel;

public:
   /**
    * Create an AppPluginFactory instance.
    *
    * @param name the name for this MicroKernelModule.
    * @param version the version for this MicroKernelModule (major.minor).
    */
   AppPluginFactory(const char* name, const char* version);

   /**
    * Deconstruct this AppPluginFactory instance.
    */
   virtual ~AppPluginFactory();

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
    * Creates an AppPlugin object.
    *
    * @return the created AppPlugin object or NULL if an exception occurred.
    */
   virtual AppPlugin* createAppPlugin() = 0;

   /**
    * Destroys an AppPlugin object.
    *
    * @param plugin the AppPlugin to destroy.
    */
   virtual void destroyAppPlugin(AppPlugin* plugin);
};

/**
 * Free an AppPlugin factory. Common case that just deletes the module.
 */
void freeAppPluginFactory(monarch::modest::Module* m);

} // end namespace app
} // end namespace monarch

#endif
