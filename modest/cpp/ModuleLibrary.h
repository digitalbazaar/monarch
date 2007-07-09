/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef ModuleLibrary_H
#define ModuleLibrary_H

#include "Object.h"
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
};

} // end namespace modest
} // end namespace db
#endif
