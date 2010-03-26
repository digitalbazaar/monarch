/*
 * Copyright (c) 2010 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/app/AppPluginFactory.h"

using namespace std;
using namespace monarch::app;
using namespace monarch::config;
using namespace monarch::kernel;
using namespace monarch::modest;
using namespace monarch::rt;

AppPluginFactory::AppPluginFactory(const char* name, const char* version) :
   MicroKernelModule(name, version)
{
   mInfo["name"] = name;
   mInfo["version"] = version;
   mInfo["type"] = "monarch.app.AppPluginFactory";
   mInfo["dependencies"]->setType(Array);
}

AppPluginFactory::~AppPluginFactory()
{
}

void AppPluginFactory::addDependency(const char* name, const char* version)
{
   DynamicObject dep;
   dep["name"] = name;
   dep["version"] = version;
   mInfo["dependencies"]->append(dep);
}

DynamicObject AppPluginFactory::getDependencyInfo()
{
   return mInfo;
}

bool AppPluginFactory::initialize(MicroKernel* k)
{
   mMicroKernel = k;
   return true;
}

void AppPluginFactory::cleanup(MicroKernel* k)
{
}

MicroKernelModuleApi* AppPluginFactory::getApi(MicroKernel* k)
{
   return this;
}

void monarch::app::freeAppPluginFactory(Module* m)
{
   delete m;
}
