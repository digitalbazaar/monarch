/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "Kernel.h"

using namespace db::modest;

Kernel::Kernel()
{
   // create engine and module library
   mEngine = new Engine();
   mModuleLibrary = new ModuleLibrary(this);
}

Kernel::~Kernel()
{
   delete mEngine;
   delete mModuleLibrary;
}

Engine* Kernel::getEngine()
{
   return mEngine;
}

ModuleLibrary* Kernel::getModuleLibrary()
{
   return mModuleLibrary;
}

inline Kernel* createModestKernel()
{
   return new db::modest::Kernel();
}

inline void freeModestKernel(Kernel* k)
{
   delete k;
}
