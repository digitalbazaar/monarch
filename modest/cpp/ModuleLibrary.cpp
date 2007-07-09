/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "ModuleLibrary.h"

using namespace std;
using namespace db::modest;
using namespace db::rt;

ModuleLibrary::ModuleLibrary()
{
}

ModuleLibrary::~ModuleLibrary()
{
}

Exception* ModuleLibrary::loadModule(const string& filename)
{
   // FIXME:
}

ModuleInterface* ModuleLibrary::getModuleInterface(const string& name)
{
   // FIXME:
}
