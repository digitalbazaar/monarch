/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/modest/Kernel.h"

using namespace monarch::modest;
using namespace monarch::rt;

Kernel::Kernel()
{
   // create engine and module library
   mEngine = new Engine();
   mModuleLibrary = new ModuleLibrary(this);
   mVersion = "1.0";
}

Kernel::~Kernel()
{
   delete mEngine;
   delete mModuleLibrary;
}

void Kernel::runOperation(Operation& op)
{
   // queue operation with engine
   getEngine()->queue(op);
}

Engine* Kernel::getEngine()
{
   return mEngine;
}

ModuleLibrary* Kernel::getModuleLibrary()
{
   return mModuleLibrary;
}

const char* Kernel::getVersion()
{
   return mVersion;
}

Kernel* createModestKernel()
{
   return new monarch::modest::Kernel();
}

void freeModestKernel(Kernel* k)
{
   delete k;
}
