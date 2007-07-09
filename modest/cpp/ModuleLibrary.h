/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_modest_ModuleLibrary_H
#define db_modest_ModuleLibrary_H

#include "Exception.h"
#include "ModuleLoader.h"

namespace db
{
namespace modest
{

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
 * @author Dave Longley
 */
class ModuleLibrary : public virtual db::rt::Object
{
protected:
   /**
    * The ModuleLoader that is used to load and unload Modules.
    */
   ModuleLoader mLoader;
   
public:
   /**
    * Creates a new ModuleLibrary.
    */
   ModuleLibrary();
   
   /**
    * Destructs this ModuleLibrary.
    */
   virtual ~ModuleLibrary();
   
   /**
    * Loads a Module into this ModuleLibrary, if it has not already been loaded.
    * 
    * @param filename the name of the file where the Module resides.
    * 
    * @return an Exception if the Module could not be loaded.
    */
   virtual db::rt::Exception* loadModule(const std::string& filename);
   
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
