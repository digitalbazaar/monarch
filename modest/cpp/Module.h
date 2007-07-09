/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef Module_H
#define Module_H

#include "ModuleInterface.h"

namespace db
{
namespace modest
{

/**
 * A Module is any extension to Modest. It can be loaded into an instance of
 * Modest run its available Operations. It can also create and provide new
 * Operations for other Modules to run.
 * 
 * @author Dave Longley
 */
class Module
{
public:
   /**
    * Creates a new Module.
    */
   Module() {};
   
   /**
    * Destructs this Module.
    */
   virtual ~Module() {};
   
   /**
    * Gets the interface for this Module. The returned object should be
    * cast to the appropriate extended ModuleInterface class for this Module.
    * 
    * @return the interface that provides access to this Module's functionality.
    */
   virtual ModuleInterface* getInterface() = 0;
};

} // end namespace modest
} // end namespace db
#endif
