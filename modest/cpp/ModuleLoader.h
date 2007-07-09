/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_modest_ModuleLoader_H
#define db_modest_ModuleLoader_H

#include "Module.h"

namespace db
{
namespace modest
{

typedef struct ModuleInfo
{
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
   virtual ModuleInfo* loadModule(std::string const& filename);
   
   /**
    * Unloads the passed Module.
    * 
    * @param m the Module to unload.
    */
   virtual void unloadModule(ModuleInfo* mi);
};

} // end namespace modest
} // end namespace db
#endif
