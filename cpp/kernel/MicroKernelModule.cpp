/*
 * Copyright (c) 2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/kernel/MicroKernelModule.h"

#include "monarch/kernel/MicroKernel.h"

using namespace monarch::kernel;
using namespace monarch::modest;
using namespace monarch::rt;

MicroKernelModule::MicroKernelModule(const char* name, const char* version)
{
   mId.name = name;
   mId.version = version;
}

MicroKernelModule::~MicroKernelModule()
{
}

const ModuleId& MicroKernelModule::getId()
{
   return mId;
}

bool MicroKernelModule::initialize(Kernel* k)
{
   // FIXME: consider renaming initialize/cleanup() here or in modest or both
   // to help eliminate confusion over what gets call when

   // nothing to do for modest initialization
   return true;
}

void MicroKernelModule::cleanup(Kernel* k)
{
   // no modest clean up to do
}

ModuleInterface* MicroKernelModule::getInterface()
{
   return NULL;
}
