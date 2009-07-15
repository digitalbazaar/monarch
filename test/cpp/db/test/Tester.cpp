/*
 * Copyright (c) 2008-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "db/test/Tester.h"

#include <iostream>
#include <sstream>
#include <cstdlib>

#include "db/test/Test.h"
#include "db/rt/Exception.h"
#include "db/rt/Thread.h"

using namespace std;
using namespace db::app;
using namespace db::config;
using namespace db::rt;
using namespace db::test;

Tester::Tester()
{
}

Tester::~Tester()
{
   for(list<Tester*>::iterator i = mTesters.begin();
      i != mTesters.end();
      i++)
   {
      delete *i;
   }
}

DynamicObject Tester::getCommandLineSpecs()
{
   DynamicObject spec;
   spec["help"] =
"Test options:\n"
"  -l, --level LEVEL   Adjust test output level to LEVEL. (default: 3)\n"
"                         0: No output.\n"
"                         1: Final results.\n"
"                         2: Progress (.=success, W=warning, F=failure).\n"
"                         3: Test names and PASS/WARNING/FAIL status.\n"
"                         4: Same as 3, plus test time.\n"
"                      All levels have exit status of 0 on success.\n"
"  -c                  Continue after failure. (default: true).\n"
"  -i, --interactive   Do only interactive tests. (default: false).\n"
"  -a, --automatic     Do only automatic tests. (default: true).\n"
"                      Note: -i and -a can be combined to do both types.\n"
"  -t, --test TEST     Run a specific test if supported. (default: \"all\")\n"
"\n";
   
   DynamicObject opt;
   
   opt = spec["options"]->append();
   opt["short"] = "-l";
   opt["long"] = "--level";
   opt["arg"]["config"] = "db.test.Tester command line";
   opt["arg"]["path"] = "db\\.test\\.Tester.level";
  
   opt = spec["options"]->append();
   opt["short"] = "-c";
   opt["setTrue"]["config"] = "db.test.Tester command line";
   opt["setTrue"]["path"] = "db\\.test\\.Tester.continueAfterException";
  
   opt = spec["options"]->append();
   opt["short"] = "-a";
   opt["long"] = "--automatic";
   opt["setTrue"]["config"] = "db.test.Tester command line";
   opt["setTrue"]["path"] = "db\\.test\\.Tester.__cl_automatic";
  
   opt = spec["options"]->append();
   opt["short"] = "-i";
   opt["long"] = "--interactive";
   opt["setTrue"]["config"] = "db.test.Tester command line";
   opt["setTrue"]["path"] = "db\\.test\\.Tester.__cl_interactive";
  
   opt = spec["options"]->append();
   opt["short"] = "-t";
   opt["long"] = "--test";
   opt["arg"]["config"] = "db.test.Tester command line";
   opt["arg"]["path"] = "db\\.test\\.Tester.test";
   
   //DynamicObject specs = AppDelegate::getCommandLineSpecs();
   // don't read App superclass options
   // FIXME need to break up base App and DbApp
   DynamicObject specs;
   specs->setType(Array);
   specs->append(spec);
   return specs;
}

bool Tester::willParseCommandLine(std::vector<const char*>* args)
{
   bool rval = App::willParseCommandLine(args);
   
   if(rval)
   {
      // set defaults
      if(rval)
      {
         Config config;
         config->setType(Map);
         config[ConfigManager::ID] = "db.test.Tester defaults";
         config[ConfigManager::GROUP] = "boot";
         config[ConfigManager::VERSION] = DB_DEFAULT_CONFIG_VERSION;
      
         Config& merge = config[ConfigManager::MERGE];
         merge["db.test.Tester"]["level"] = TestRunner::Names;
         merge["db.test.Tester"]["continueAfterException"] = false;
         merge["db.test.Tester"]["test"] = "all";
      
         rval = getConfigManager()->addConfig(config);
      }
   
      // config to hold potential command line options
      if(rval)
      {
         Config config;
         config->setType(Map);
         config[ConfigManager::ID] = "db.test.Tester command line";
         config[ConfigManager::PARENT] = getParentOfMainConfigGroup();
         config[ConfigManager::GROUP] = getMainConfigGroup();
         config[ConfigManager::VERSION] = DB_DEFAULT_CONFIG_VERSION;
         rval = getConfigManager()->addConfig(config);
      }
   }
   
   return rval;
}

bool Tester::didParseCommandLine()
{
   bool rval = App::didParseCommandLine();
   
   if(rval)
   {
      // to get values set on command line
      Config rawConfig = getConfigManager()->getConfig(
         "db.test.Tester command line", true);
      Config& config = rawConfig[ConfigManager::MERGE]["db.test.Tester"];
   
      // if interactive, assume no automatic, else only automatic enabled
      if(config->hasMember("__cl_interactive") &&
         config["__cl_interactive"]->getBoolean())
      {
         config["interactive"] = true;
         config["automatic"] = false;
      }
      else
      {
         config["interactive"] = false;
         config["automatic"] = true;
      }
      
      // if auto set, override interactive setting
      if(config->hasMember("__cl_automatic") &&
         config["__cl_automatic"]->getBoolean())
      {
         config["automatic"] = true;
      }
      
      rval = getConfigManager()->setConfig(rawConfig);
   }
   
   return rval;
}

void Tester::setup(TestRunner& tr)
{
}

void Tester::teardown(TestRunner& tr)
{
}

void Tester::addTester(Tester* tester)
{
   tester->setOwner(this);
   mTesters.push_back(tester);
}

int Tester::runAutomaticTests(TestRunner& tr)
{
   return 0;
}

int Tester::runInteractiveTests(TestRunner& tr)
{
   return 0;
}

int Tester::runTests(TestRunner& tr)
{
   int rval = 0;
   Config cfg = getConfig()["db.test.Tester"];

   tr.group(mName);

   setup(tr);
   assertNoException();

   // run all sub-tester tests
   for(list<Tester*>::iterator i = mTesters.begin();
      rval == 0 && i != mTesters.end();
      i++)
   {
      rval = (*i)->runTests(tr);
   }

   if(rval == 0 && cfg["interactive"]->getBoolean())
   {
      rval = runInteractiveTests(tr);
      assertNoException();
   }

   if(rval == 0 && cfg["automatic"]->getBoolean())
   {
      rval = runAutomaticTests(tr);
      assertNoException();
   }
   
   teardown(tr);
   assertNoException();

   tr.ungroup();
   
   return rval;
}

bool Tester::runApp()
{
   bool rval = true;
   
   Config cfg = getConfig()["db.test.Tester"];
   bool cont = cfg["continueAfterException"]->getBoolean();
   uint32_t cfgLevel = cfg["level"]->getUInt32();
   TestRunner::OutputLevel level;
   
   switch(cfgLevel)
   {
      case 0: level = TestRunner::None; break;
      case 1: level = TestRunner::Final; break;
      case 2: level = TestRunner::Progress; break;
      case 3: level = TestRunner::Names; break;
      default: level = TestRunner::Times; break;
   }
   
   App* app = getOwner();
   app = (app != NULL) ? app : this;

   TestRunner tr(app, cont, level);
   
   int exitStatus = runTests(tr);
   app->setExitStatus(exitStatus);
   rval = (exitStatus == 0);
   assertNoException();
   
   tr.done();
   
   return rval;
}
