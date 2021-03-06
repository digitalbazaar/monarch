/*
 * Copyright (c) 2007-2010 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_modest_ModuleLoader_H
#define monarch_modest_ModuleLoader_H

#include "monarch/modest/Module.h"

namespace monarch
{
namespace modest
{

/**
 * Stores information about a Module.
 */
struct ModuleInfo
{
   /**
    * The handle to the Module.
    */
   void* handle;

   /**
    * The Module this info is about.
    */
   Module* module;

   /**
    * The free function for this Module.
    */
   FreeModestModuleFn freeModule;
};

/**
 * A ModuleLoader is used to load Modules from files.
 *
 * @author Dave Longley
 */
class ModuleLoader
{
public:
   /**
    * Creates a new ModuleLoader.
    */
   ModuleLoader();

   /**
    * Destructs this ModuleLoader.
    */
   virtual ~ModuleLoader();

   /**
    * Loads a Module from a file with the given filename. The allocated
    * ModuleInfo will be freed when unloadModule(ModuleInfo*) is called.
    *
    * @param filename the name of the file with the Module.
    *
    * @return the loaded Module's info or NULL if an Exception occurred.
    */
   virtual ModuleInfo* loadModule(const char* filename);

   /**
    * Loads a Module using the given functions. The allocated ModuleInfo
    * will be freed when unloadModule(ModuleInfo*) is called.
    *
    * @param cm the create module function.
    * @param fm the free module function.
    *
    * @return the loaded Module's info or NULL if an Exception occurred.
    */
   virtual ModuleInfo* loadModule(
      CreateModestModuleFn cm, FreeModestModuleFn fm);

   /**
    * Unloads the passed Module.
    *
    * @param m the Module to unload.
    */
   virtual void unloadModule(ModuleInfo* mi);
};

} // end namespace modest
} // end namespace monarch
#endif
