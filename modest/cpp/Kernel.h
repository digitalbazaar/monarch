/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef Kernel_H
#define Kernel_H

#include "Engine.h"
#include "ModuleLibrary.h"

namespace db
{
namespace modest
{

/**
 * A Kernel maintains an Engine and a ModuleLibrary with Modules that extend
 * that Engine's functionality. It is the upper-most layer of the Modest
 * system and provides a public interface for applications (i.e. GUIs) to
 * make use of the Modest Engine and its Modules.
 * 
 * @author Dave Longley
 */
class Kernel
{
protected:
   /**
    * The Engine for this Kernel.
    */
   Engine* mEngine;
   
   /**
    * The ModuleLibrary for this Kernel.
    */
   ModuleLibrary* mModuleLibrary;
   
public:
   /**
    * Creates a new Kernel.
    */
   Kernel();
   
   /**
    * Destructs this Kernel.
    */
   virtual ~Kernel();
   
   /**
    * Gets this Kernel's Engine.
    * 
    * @return this Kernel's Engine.
    */
   virtual Engine* getEngine();
   
   /**
    * Gets this Kernel's ModuleLibrary.
    * 
    * @return this Kernel's ModuleLibrary.
    */
   virtual ModuleLibrary* getModuleLibrary();
};

} // end namespace modest
} // end namespace db
#endif
