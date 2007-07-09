/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_modest_ModuleLibrary_H
#define db_modest_ModuleLibrary_H

#include "Exception.h"
#include "ModuleLoader.h"

#include <map>

namespace db
{
namespace modest
{

// forward declare Kernel
class Kernel;

/**
 * A ModuleLibrary is used to maintain a list of loaded Modules and to allow
 * new Modules to register and unregister themselves. As long as a ModuleLibrary
 * is in memory, the Modules that have registered with it will remain loaded
 * unless they are unregistered. When this ModuleLibrary is destructed, the
 * Modules it has loaded will be unloaded.
 * 
 * A ModuleLibrary can be used to look up registered Modules and their
 * available Operations.
 * 
 * This class provides no dependency checking -- it is left up to the Modules
 * to check for their own dependencies and up to the application to ensure
 * that Modules get unloaded in a safe order, if they are unloaded while
 * the Modest Engine is running. 
 * 
 * @author Dave Longley
 */
class ModuleLibrary : public virtual db::rt::Object
{
protected:
   /**
    * The Kernel this library is for.
    */
   Kernel* mKernel;
   
   /**
    * The ModuleLoader that is used to load Modules from files.
    */
   ModuleLoader mLoader;
   
   /**
    * The map of loaded Modules.
    */
   std::map<std::string, ModuleInfo*> mModules;
   
   /**
    * Finds a loaded Module by its name.
    */
   Module* findModule(const std::string& name);
   
public:
   /**
    * Creates a new ModuleLibrary for the specified Kernel.
    * 
    * @param k the Kernel this ModuleLibrary is for.
    */
   ModuleLibrary(Kernel* k);
   
   /**
    * Destructs this ModuleLibrary.
    */
   virtual ~ModuleLibrary();
   
   /**
    * Loads a Module into this ModuleLibrary, if it has not already been loaded.
    * 
    * @param filename the name of the file where the Module resides.
    * 
    * @return true if the Module was loaded, false if not.
    */
   virtual bool loadModule(const std::string& filename);
   
   /**
    * Unloads a Module from this ModuleLibrary, if it is loaded.
    * 
    * @param name the name of the Module to unload.
    */
   virtual void unloadModule(const std::string& name);
   
   /**
    * Gets the ModuleId for the Module with the given name.
    * 
    * @return the ModuleId for the Module with the given name or NULL if one
    *         does not exist.
    */
   virtual const ModuleId* getModuleId(const std::string& name);
   
   /**
    * Gets the interface to the Module with the given name.
    * 
    * @param name the name of the Module to get the interface for.
    * 
    * @return the Module's interface, or NULL if the Module does not exist
    *         or it has no interface.
    */
   virtual ModuleInterface* getModuleInterface(const std::string& name);
};

} // end namespace modest
} // end namespace db
#endif
