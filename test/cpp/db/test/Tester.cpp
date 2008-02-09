/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/test/Tester.h"

#include <assert.h>
#include <iostream>
#include <sstream>

#include "db/test/Test.h"
#include "db/rt/Exception.h"
#include "db/rt/Thread.h"

using namespace std;
using namespace db::rt;
using namespace db::test;

Tester::Tester()
{
   mExitStatus = 0;
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

void Tester::run()
{
   TestRunner tr(true, TestRunner::Names);
   
   mExitStatus = runTests(tr);
   assertNoException();
   
   tr.done();
}

void Tester::loggingInitialize()
{
}

void Tester::loggingCleanup()
{
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
   
   loggingInitialize();
   
   Thread t(this);
   t.start(131072);
   t.join();
   
   loggingCleanup();
   
   // cleanup winsock
   #ifdef WIN32
      WSACleanup();
   #endif
   
   ExceptionRef e = new Exception("Main thread exception leak test");
   Exception::setLast(e, false);
   
   #ifndef WIN32
   // FIXME: calling Thread::exit() on windows causes a busy loop of
   // some sort (perhaps a deadlock spin lock)
   Thread::exit();
   #endif
   
   return mExitStatus;
}
