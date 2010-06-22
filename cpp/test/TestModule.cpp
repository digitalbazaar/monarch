/*
 * Copyright (c) 2010 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/test/TestModule.h"

using namespace std;
using namespace monarch::app;
using namespace monarch::config;
using namespace monarch::kernel;
using namespace monarch::modest;
using namespace monarch::rt;
using namespace monarch::test;

TestModule::TestModule(const char* name, const char* version) :
   MicroKernelModule(name, version),
      mRun(NULL)
{
   mInfo["name"] = name;
   mInfo["version"] = version;
   mInfo["type"] = "monarch.test.TestModule";
   mInfo["dependencies"]->setType(Array);
}

TestModule::~TestModule()
{
}

void TestModule::addDependency(const char* name, const char* version)
{
   DynamicObject dep;
   dep["name"] = name;
   dep["version"] = version;
   mInfo["dependencies"]->append(dep);
}

DynamicObject TestModule::getDependencyInfo()
{
   return mInfo;
}

bool TestModule::initialize(MicroKernel* k)
{
   return true;
}

void TestModule::cleanup(MicroKernel* k)
{
}

MicroKernelModuleApi* TestModule::getApi(MicroKernel* k)
{
   return this;
}

void TestModule::setTestFunction(RunTestRunnerFn fn)
{
   mRun = fn;
}

bool TestModule::run(TestRunner& tr)
{
   return (mRun != NULL) ? mRun(tr) : true;
}
