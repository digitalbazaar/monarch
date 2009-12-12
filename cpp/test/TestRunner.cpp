/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/test/TestRunner.h"

#include <cassert>
#include <iostream>

#include "monarch/test/Test.h"
#include "monarch/rt/Exception.h"

using namespace std;
using namespace monarch::rt;
using namespace monarch::test;

TestRunner::TestRunner(
   monarch::app::App* app, bool doneOnException, OutputLevel outputLevel)
{
   mApp = app;
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

monarch::app::App* TestRunner::getApp()
{
   return mApp;
}

TestRunner::OutputLevel TestRunner::getOutputLevel()
{
   return mOutputLevel;
}

/**
 * Map to convert output level option names to TestRunner::OutputLevel types
 */
struct outputLevelMap {
   const char* key;
   TestRunner::OutputLevel level;
};
static const struct outputLevelMap outputLevelsMap[] = {
   {"none", TestRunner::None},
   {"f", TestRunner::Final},
   {"final", TestRunner::Final},
   {"p", TestRunner::Progress},
   {"progress", TestRunner::Progress},
   {"n", TestRunner::Names},
   {"names", TestRunner::Names},
   {"t", TestRunner::Times},
   {"times", TestRunner::Times},
   {NULL, TestRunner::None}
};

bool TestRunner::stringToLevel(const char* slevel, OutputLevel& level)
{
   bool found = false;
   for(int mapi = 0;
      slevel != NULL&& !found && outputLevelsMap[mapi].key != NULL;
      mapi++)
   {
      if(strcasecmp(slevel, outputLevelsMap[mapi].key) == 0)
      {
         level = outputLevelsMap[mapi].level;
         found = true;
      }
   }

   return found;
}

const char* TestRunner::levelToString(OutputLevel level)
{
   const char* rval;

   switch(level)
   {
      case None:
         rval = "None";
         break;
      case Final:
         rval = "Final";
         break;
      case Progress:
         rval = "Progress";
         break;
      case Names:
         rval = "Names";
         break;
      case Times:
         rval = "Times";
         break;
      default:
         rval = NULL;
   }

   return rval;
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
   if(Exception::isSet())
   {
      fail();
      ExceptionRef e = Exception::get();
      dumpException(e);
      if(mDoneOnException)
      {
         done();
         assert(!Exception::isSet());
      }
      else
      {
         Exception::clear();
      }
   }
   else
   {
      pass();
   }
}

void TestRunner::passIfException(bool dump)
{
   if(Exception::isSet())
   {
      if(dump)
      {
         dumpException();
      }
      Exception::clear();
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
