/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/modest/Kernel.h"
#include "db/modest/OperationImpl.h"

using namespace db::modest;
using namespace db::rt;

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

Operation Kernel::createOperation(
   Runnable* r, OperationGuard* g, StateMutator* m)
{
   // create normal operation
   return Operation(new OperationImpl(r, g, m));
}

Operation Kernel::createOperation(
   CollectableRunnable& r, OperationGuard* g, StateMutator* m)
{
   // create normal operation
   return Operation(new OperationImpl(r, g, m));
}

void Kernel::runOperation(Operation op)
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
