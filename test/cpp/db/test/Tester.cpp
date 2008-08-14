/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
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
   mName = NULL;
   setName("");
   mApp = NULL;
}

Tester::~Tester()
{
   setName(NULL);
   for(list<Tester*>::iterator i = mTesters.begin();
      i != mTesters.end();
      i++)
   {
      delete *i;
   }
}

void Tester::registeredForApp(db::app::App* app)
{
   mApp = app;
   for(list<Tester*>::iterator i = mTesters.begin();
      i != mTesters.end();
      i++)
   {
      (*i)->registeredForApp(mApp);
   }
}

DynamicObject Tester::getCommandLineSpec()
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
   Config& cfg = mApp->getConfig(); 
   
   opt = spec["options"]->append();
   opt["short"] = "-l";
   opt["long"] = "--level";
   opt["arg"] = cfg["db.test.Tester"]["level"];
  
   opt = spec["options"]->append();
   opt["short"] = "-c";
   opt["setTrue"] = cfg["db.test.Tester"]["continueAfterException"];
  
   opt = spec["options"]->append();
   opt["short"] = "-a";
   opt["long"] = "--automatic";
   opt["setTrue"] = cfg["db.test.Tester"]["__cl_automatic"];
  
   opt = spec["options"]->append();
   opt["short"] = "-i";
   opt["long"] = "--interactive";
   opt["setTrue"] = cfg["db.test.Tester"]["__cl_interactive"];
  
   opt = spec["options"]->append();
   opt["short"] = "-t";
   opt["long"] = "--test";
   opt["arg"] = cfg["db.test.Tester"]["test"];
  
   return spec;
}

bool Tester::willParseCommandLine(std::vector<const char*>* args)
{
   bool rval = true;
   
   mApp->getConfig()["db.test.Tester"]["level"] = TestRunner::Names;
   mApp->getConfig()["db.test.Tester"]["continueAfterException"] = false;
   mApp->getConfig()["db.test.Tester"]["test"] = "all";
   
   return rval;
}

bool Tester::didParseCommandLine()
{
   bool rval = true;
   Config& cfg = mApp->getConfig()["db.test.Tester"];

   // if interactive, assume no automatic, else only automatic enabled
   if(cfg->hasMember("__cl_interactive") &&
      cfg["__cl_interactive"]->getBoolean())
   {
      cfg["interactive"] = true;
      cfg["automatic"] = false;
   }
   else
   {
      cfg["interactive"] = false;
      cfg["automatic"] = true;
   }
   
   // if auto set, override interactive setting
   if(cfg->hasMember("__cl_automatic") &&
      cfg["__cl_automatic"]->getBoolean())
   {
      cfg["automatic"] = true;
   }
   
   return rval;
}

void Tester::setName(const char* name)
{
   if(mName)
   {
      free(mName);
   }
   mName = name ? strdup(name) : NULL;
}

const char* Tester::getName()
{
   return mName;
}

db::config::Config& Tester::getConfig()
{
   return mApp->getConfig();
}

void Tester::setup(TestRunner& tr)
{
}

void Tester::teardown(TestRunner& tr)
{
}

void Tester::addTester(Tester* tester)
{
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
   Config& cfg = mApp->getConfig()["db.test.Tester"];

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
   
   Config& cfg = mApp->getConfig()["db.test.Tester"];
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
   
   TestRunner tr(mApp, cont, level);
   
   int exitStatus = runTests(tr);
   mApp->setExitStatus(exitStatus);
   rval = (exitStatus == 0);
   assertNoException();
   
   tr.done();
   
   return rval;
}
