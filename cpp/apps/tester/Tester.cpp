/*
 * Copyright (c) 2008-2010 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/apps/tester/Tester.h"

#include "monarch/app/AppFactory.h"
#include "monarch/kernel/MicroKernel.h"
#include "monarch/rt/Exception.h"
#include "monarch/test/Test.h"
#include "monarch/test/Testable.h"
#include "monarch/test/TestLoader.h"

#include <cstdio>
#include <cstdlib>

using namespace std;
using namespace monarch::app;
using namespace monarch::apps::tester;
using namespace monarch::config;
using namespace monarch::kernel;
using namespace monarch::modest;
using namespace monarch::rt;
using namespace monarch::test;

#define APP_NAME "monarch.apps.tester.Tester"

Tester::Tester()
{
   setName(APP_NAME);
   setVersion("1.0");
}

Tester::~Tester()
{
}

bool Tester::initConfigs(Config& defaults)
{
   // add test loader defaults
   TestLoader loader;
   bool rval =
      loader.initConfigs(defaults) &&
      getConfigManager()->addConfig(defaults);
   return rval;
}

DynamicObject Tester::getCommandLineSpec(Config& cfg)
{
   // add test loader command line options
   TestLoader loader;
   return loader.getCommandLineSpec(cfg);
}

bool Tester::run()
{
   // use test loader to run tests
   TestLoader loader;
   return loader.run(this);
}

class TesterFactory : public AppFactory
{
public:
   TesterFactory() : AppFactory(APP_NAME, "1.0")
   {
   }

   virtual ~TesterFactory() {}

   virtual App* createApp()
   {
      return new Tester();
   }
};

Module* createModestModule()
{
   return new TesterFactory();
}

void freeModestModule(Module* m)
{
   delete m;
}
