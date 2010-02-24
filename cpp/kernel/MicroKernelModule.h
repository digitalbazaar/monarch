/*
 * Copyright (c) 2010 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_kernel_MicroKernelModule_H
#define monarch_kernel_MicroKernelModule_H

#include "monarch/kernel/MicroKernelModuleApi.h"
#include "monarch/modest/Module.h"

namespace monarch
{
namespace kernel
{

class MicroKernel;

/**
 * A MicroKernelModule is an abstract modest Module that provides dependency
 * information about other related MicroKernelModules.
 *
 * A MicroKernel may have many different modules that it loads that depend upon
 * each other in different ways, and a MicroKernelModule allows its MicroKernel
 * to initialize them in the proper order. A MicroKernelModule also provides
 * access to the MicroKernel that is loading a given module at initialization
 * and clean up time.
 *
 * In order to provide the API for whatever new functionality the module
 * implements, the MicroKernelModuleApi class should be extended. The typical
 * design pattern involves creating an abstract virtual class that extends
 * MicroKernelModuleApi that allows other modules can dynamically cast the
 * return value of getApi() to. This also enables those modules to avoid having
 * to link against the module with the API they wish to use. They should only
 * need the applicable header.
 *
 * Any modest module that is meant to be loaded by a MicroKernel should extend
 * this class or another class that also extends this class. There are several
 * abstract methods that need implementation. They will provide the dependency
 * information, do custom initialization and clean up, and provide the API,
 * if any, for this module so that other modules can take advantage of its
 * functionality. If a module has no API (it may not need one or it may only
 * provide a web-API), then it should return NULL from the getApi() method.
 *
 * The order of loading a MicroKernelModule is as follows:
 *
 * Modest Layer:
 * 1. Modest Kernel loads a module as a Modest module.
 * 2. Modest Kernel calls initialize(monarch::modest::Kernel*).
 * 3. On success, Modest Kernel gives control over to the MicroKernel Layer.
 *
 * MicroKernel Layer:
 * 1. MicroKernel gets dependency information from the MicroKernelModuleApi.
 * 2. MicroKernel resolves dependency information and loads and initializes any
 *    other MicroKernelModules the module depends on.
 * 3. MicroKernel calls initialize(monarch::kernel::MicroKernel*).
 *
 * @author Dave Longley
 */
class MicroKernelModule : public monarch::modest::Module
{
protected:
   /**
    * The ID for this module.
    */
   monarch::modest::ModuleId mId;

public:
   /**
    * Creates a new MicroKernelModule with the specified name and version.
    *
    * @param name the name for this MicroKernelModule.
    * @param version the version for this MicroKernelModule (major.minor).
    */
   MicroKernelModule(const char* name, const char* version);

   /**
    * Destructs this MicroKernelModule.
    */
   virtual ~MicroKernelModule();

   /**
    * Gets the ID of this Module.
    *
    * @return the ID of this Module.
    */
   virtual const monarch::modest::ModuleId& getId();

   /**
    * Initializes this Module with the modest Kernel once it has been loaded.
    *
    * This call will automatically be made by the lower-level modest Kernel to
    * initialize any special basic functionality that does not depend on other
    * modules. Typically, this method does not need to be overwritten when
    * writing a new MicroKernelModule. Instead, the initialize(MicroKernel*)
    * call that takes a MicroKernel must be implemented. That call will
    * automatically be made after this call and after any dependencies are
    * resolved. That call will provide direct access to the MicroKernel and its
    * capabilities.
    *
    * @param k the the modest Kernel to initialize with.
    *
    * @return true if the module initialized, false if not (with
    *         an Exception set).
    */
   virtual bool initialize(monarch::modest::Kernel* k);

   /**
    * Cleans up this Module just prior to its unloading. This will
    * automatically be called after cleanup(MicroKernel*).
    *
    * @param k the modest Kernel that is unloading this Module.
    */
   virtual void cleanup(monarch::modest::Kernel* k);

   /**
    * Gets the modest module interface for this Module. A MicroKernelModule
    * has no modest module interface, it may only have a MicroKernelModuleApi
    * that is accessible via getApi() after initialization with the MicroKernel.
    *
    * @return NULL.
    */
   virtual monarch::modest::ModuleInterface* getInterface();

   /**
    * Gets dependency information.
    *
    * @return the dependency information.
    */
   virtual monarch::rt::DynamicObject getDependencyInfo() = 0;

   /**
    * Initializes this MicroKernelModule with the passed MicroKernel. This call
    * will be made after the lower-level initialize(Kernel*) call and after any
    * other MicroKernelModule dependencies have been resolved.
    *
    * @param k the MicroKernel.
    *
    * @return true if initialized, false if an Exception occurred.
    */
   virtual bool initialize(MicroKernel* k) = 0;

   /**
    * Cleans up this MicroKernelModule just prior to its unloading.
    *
    * @param k the MicroKernel.
    */
   virtual void cleanup(MicroKernel* k) = 0;

   /**
    * Gets the API for this MicroKernelModule.
    *
    * @param k the MicroKernel that loaded this module.
    *
    * @return the API for this MicroKernelModule.
    */
   virtual MicroKernelModuleApi* getApi(MicroKernel* k) = 0;
};

} // end namespace kernel
} // end namespace monarch
#endif
