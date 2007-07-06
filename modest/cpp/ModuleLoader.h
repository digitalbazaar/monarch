/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef ModuleLoader_H
#define ModuleLoader_H

#include "Object.h"

namespace db
{
namespace modest
{

/**
 * A ModuleLoader is used to load (and unload) Modules.
 * 
 * @author Dave Longley
 */
class ModuleLoader : public virtual db::rt::Object
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
};

} // end namespace modest
} // end namespace db
#endif
