/*
 * Copyright (c) 2007-2010 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/test/TestRunner.h"

#include "monarch/test/Test.h"
#include "monarch/rt/Exception.h"

#include <cassert>
#include <cstdio>

using namespace std;
using namespace monarch::rt;
using namespace monarch::test;

const char* TestRunner::DEFAULT = "default";

TestRunner::TestRunner(
   monarch::app::App* app, bool doneOnException, OutputLevel outputLevel) :
   mApp(app),
   mTotal(0),
   mPassed(0),
   mFailed(0),
   mWarnings(0),
   mOutputLevel(outputLevel),
   mDoneOnException(doneOnException),
   mListTests(false)
{
   enableTest(DEFAULT);
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
       ++i)
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

void TestRunner::setListTests(bool listTests)
{
   mListTests = listTests;
}

void TestRunner::enableTest(std::string test, bool enabled)
{
   mTests[test] = enabled;
}

bool TestRunner::isTestEnabled(std::string test)
{
   bool rval = false;

   if(mListTests)
   {
      printf("%s %s\n", getTestName().c_str(), test.c_str());
      rval = false;
   }
   else
   {
      std::map<string, bool>::iterator i = mTests.find(test);
      rval = (i != mTests.end()) ? i->second : false;
   }

   return rval;
}

bool TestRunner::isDefaultEnabled()
{
   return isTestEnabled(DEFAULT);
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
       ++mapi)
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
   mTestPath.push_back(string((name != NULL) ? name : "") + "/");
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
         printf("%s... ", getTestName().c_str());
         fflush(stdout);;
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
         printf(".");;
         fflush(stdout);
         break;
      case Names:
      case Times:
         printf("PASS.\n");
         fflush(stdout);
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
         printf("F");
         fflush(stdout);
         break;
      case Names:
      case Times:
         printf("FAIL.\n");
         if(reason != NULL)
         {
            printf("%s\n", reason);
         }
         fflush(stdout);
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
         printf("W");
         fflush(stdout);
         break;
      case Names:
      case Times:
         printf("WARNING: %s", (reason != NULL) ? reason : "(no reason given)");
         fflush(stdout);
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
            printf("\n");
         }
         // fall through
      case Final:
      case Names:
      case Times:
         printf("Done. Total:%u Passed:%u Failed:%u Warnings:%u Unknown:%u.\n",
            mTotal, mPassed, mFailed, mWarnings, unknown);
         break;
      default:
         break;
   }
}
