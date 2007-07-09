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

void Kernel::startEngine()
{
   mEngine->start();
}

void Kernel::stopEngine()
{
   mEngine->stop();
}

ModuleLibrary* Kernel::getModuleLibrary()
{
   return mModuleLibrary;
}
