/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/test/Tester.h"

// openssl includes
#include <openssl/ssl.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#include <openssl/engine.h>

#include <iostream>
#include <sstream>

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

DynamicObject Tester::getCommandLineSpec(App* app)
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
"\n";
   
   DynamicObject opt;
   Config& cfg = app->getConfig(); 
   
   opt = spec["options"]->append();
   opt["short"] = "-l";
   opt["arg"] = cfg["db.test.Tester"]["level"];
  
   opt = spec["options"]->append();
   opt["short"] = "-c";
   opt["setTrue"] = cfg["db.test.Tester"]["continueAfterException"];
  
   return spec;
}

bool Tester::willParseCommandLine(App* app, std::vector<const char*>* args)
{
   bool rval = true;
   
   app->getConfig()["db.test.Tester"]["level"] = 3;
   app->getConfig()["db.test.Tester"]["continueAfterException"] = false;
   
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

   if(rval == 0)
   {
      rval = runInteractiveTests(tr);
      assertNoException();
   }

   if(rval == 0)
   {
      rval = runAutomaticTests(tr);
      assertNoException();
   }
   
   teardown(tr);
   assertNoException();

   tr.ungroup();
   
   return rval;
}

void Tester::run(App* app)
{
   Config& cfg = app->getConfig();
   bool cont = cfg["db.test.Tester"]["continueAfterException"]->getBoolean();
   uint32_t cfgLevel = cfg["db.test.Tester"]["level"]->getUInt32();
   TestRunner::OutputLevel level;
   
   switch(cfgLevel)
   {
      case 0: level = TestRunner::None; break;
      case 1: level = TestRunner::Final; break;
      case 2: level = TestRunner::Progress; break;
      case 3: level = TestRunner::Names; break;
      default: level = TestRunner::Times; break;
   }
   
   TestRunner tr(cont, level);
   
   int exitStatus = runTests(tr);
   app->setExitStatus(exitStatus);
   assertNoException();
   
   tr.done();
}
