/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/test/TestRunner.h"

#include <cassert>
#include <iostream>

#include "db/test/Test.h"
#include "db/rt/Exception.h"

using namespace std;
using namespace db::rt;
using namespace db::test;

TestRunner::TestRunner(bool doneOnException, OutputLevel outputLevel)
{
   mOutputLevel = outputLevel;
   mTotal = 0;
   mPassed = 0;
   mWarnings = 0;
   mFailed = 0;
   mDoneOnException = doneOnException;
}

TestRunner::~TestRunner()
{
}

string TestRunner::getTestName()
{
   vector<string>::iterator i;
   string res;

   for(i = mTestPath.begin(); 
       i != mTestPath.end();
       i++)
   {
      res += *i;
   }

   return res;
}

void TestRunner::group(const char* name)
{
   mTestPath.push_back(string(name) + "/");
}

void TestRunner::ungroup()
{
   mTestPath.pop_back();
}

void TestRunner::test(const char* name)
{
   mTotal++;
   mTestPath.push_back(string(name));
   switch(mOutputLevel)
   {
      case Names:
      case Times:
         cout << getTestName() << "... ";
         cout.flush();
         break;
      default:
         break;
   }
   mTestPath.pop_back();
}

void TestRunner::pass()
{
   mPassed++;
   switch(mOutputLevel)
   {
      case Progress:
         cout << ".";
         cout.flush();
         break;
      case Names:
      case Times:
         cout << "PASS." << endl;
         cout.flush();
         break;
      default:
         break;
   }
}

void TestRunner::passIfNoException()
{
   if(Exception::hasLast())
   {
      fail();
      ExceptionRef e = Exception::getLast();
      dumpException(e);
      if(mDoneOnException)
      {
         done();
         assert(!Exception::hasLast());
      }
   }
   else
   {
      pass();
   }
}

void TestRunner::passIfException(bool dump)
{
   if(Exception::hasLast())
   {
      if(dump)
      {
         dumpException();
      }
      Exception::clearLast();
      pass();
   }
   else
   {
      // FIXME throw failure with expected exception
      fail();
   }
}

void TestRunner::fail(const char* reason)
{
   mFailed++;
   switch(mOutputLevel)
   {
      case Progress:
         cout << "F";
         cout.flush();
         break;
      case Names:
      case Times:
         cout << "FAIL." << endl;
         if(reason != NULL)
         {
            cout << reason << endl;
         }
         cout.flush();
         break;
      default:
         break;
   }
}

void TestRunner::warning(const char* reason)
{
   mWarnings++;
   switch(mOutputLevel)
   {
      case Progress:
         cout << "W";
         cout.flush();
         break;
      case Names:
      case Times:
         cout << "WARNING: ";
         if(reason != NULL)
         {
            cout << reason << endl;
         }
         else
         {
            cout << "(no reason given)" << endl;
         }
         cout.flush();
         break;
      default:
         break;
   }
}

void TestRunner::done()
{
   int unknown = mTotal - mPassed - mFailed;

   // Check that totals are sane.  Likely failure is mismatched
   // group/ungroup or test/pass/fail.
   assert(unknown >= 0);

   switch(mOutputLevel)
   {
      case Progress:
         // Progress just prints chars so force a newline if tests were done
         if(mTotal > 0)
         {
            cout << endl;
         }
         // fall through
      case Final:
      case Names:
      case Times:
         cout << "Done.";
         cout << " Total:" << mTotal;
         cout << " Passed:" << mPassed;
         cout << " Failed:" << mFailed;
         cout << " Warnings:" << mWarnings;
         cout << " Unknown:" << unknown;
         cout << "." << endl;
         break;
      default:
         break;
   }
}
