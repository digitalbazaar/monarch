/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "Kernel.h"

using namespace std;
using namespace db::modest;

Kernel::Kernel()
{
   // create engine and module library
   mEngine = new Engine();
   mModuleLibrary = new ModuleLibrary();
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
