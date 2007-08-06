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
   mVersion = "1.0";
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

const std::string& Kernel::getVersion()
{
   return mVersion;
}

inline Kernel* createModestKernel()
{
   return new db::modest::Kernel();
}

inline void freeModestKernel(Kernel* k)
{
   delete k;
}
