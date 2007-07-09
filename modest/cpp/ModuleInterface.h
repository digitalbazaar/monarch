/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef ModuleInterface_H
#define ModuleInterface_H

namespace db
{
namespace modest
{

/**
 * A ModuleInterface defines the interface through which an application can
 * access a Module. This class should be extended to provide whatever interface
 * is available for interacting with a specific Module.
 * 
 * @author Dave Longley
 */
class ModuleInterface
{
public:
   /**
    * Creates a new ModuleInterface.
    */
   ModuleInterface() {};
   
   /**
    * Destructs this ModuleInterface.
    */
   virtual ~ModuleInterface() {};
};

} // end namespace modest
} // end namespace db
#endif
