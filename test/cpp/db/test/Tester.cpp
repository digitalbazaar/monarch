/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */

#include <assert.h>
#include <iostream>
#include <sstream>

#include "db/test/Test.h"
#include "db/test/Tester.h"
#include "db/rt/Exception.h"
#include "db/rt/Thread.h"

using namespace std;
using namespace db::rt;
using namespace db::test;
using namespace db::util;

Tester::Tester()
{
   mExitStatus = 0;
   mName = NULL;
   setName("");
}

Tester::~Tester()
{
   setName(NULL);
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
   int rval;
   rval = runInteractiveTests(tr);
   assertNoException();
   if(rval == 0)
   {
      rval = runAutomaticTests(tr);
      assertNoException();
   }
   
   return rval;
}

void Tester::run()
{
   TestRunner tr(true, TestRunner::Names);
   
   // root group
   tr.group(mName);
   mExitStatus = runTests(tr);
   assertNoException();
   tr.ungroup();
   
   tr.done();
}

int Tester::main(int argc, const char* argv[])
{
   // initialize winsock
   #ifdef WIN32
      WSADATA wsaData;
      if(WSAStartup(MAKEWORD(2, 0), &wsaData) < 0)
      {
         cout << "ERROR! Could not initialize winsock!" << endl;
      }
   #endif
   
   Thread t(this);
   t.start(131072);
   t.join();
   
   // cleanup winsock
   #ifdef WIN32
      WSACleanup();
   #endif
   
   Exception::setLast(new Exception("Main thread exception leak test"));

   #ifndef WIN32
   // FIXME: calling Thread::exit() on windows causes a busy loop of
   // some sort (perhaps a deadlock spin lock)
   Thread::exit();
   #endif
   
   return mExitStatus;
}
