/*
 * Copyright (c) 2010 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/app/AppFactory.h"

using namespace std;
using namespace monarch::app;
using namespace monarch::config;
using namespace monarch::kernel;
using namespace monarch::modest;
using namespace monarch::rt;

AppFactory::AppFactory(const char* name, const char* version) :
   MicroKernelModule(name, version),
   mMicroKernel(NULL)
{
   mInfo["name"] = name;
   mInfo["version"] = version;
   mInfo["type"] = "monarch.app.AppFactory";
   mInfo["dependencies"]->setType(Array);
}

AppFactory::~AppFactory()
{
}

void AppFactory::addDependency(const char* name, const char* version)
{
   DynamicObject dep;
   dep["name"] = name;
   dep["version"] = version;
   mInfo["dependencies"]->append(dep);
}

DynamicObject AppFactory::getDependencyInfo()
{
   return mInfo;
}

bool AppFactory::initialize(MicroKernel* k)
{
   mMicroKernel = k;
   return true;
}

void AppFactory::cleanup(MicroKernel* k)
{
}

MicroKernelModuleApi* AppFactory::getApi(MicroKernel* k)
{
   return this;
}

void AppFactory::destroyApp(App* app)
{
   delete app;
}

void monarch::app::freeAppFactory(Module* m)
{
   delete m;
}
