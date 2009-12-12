/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_modest_ModuleInterface_H
#define monarch_modest_ModuleInterface_H

namespace monarch
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
} // end namespace monarch
#endif
