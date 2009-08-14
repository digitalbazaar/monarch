/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef db_test_TestRunner_H
#define db_test_TestRunner_H

#include <vector>
#include <string>

#include "db/app/App.h"

namespace db
{
namespace test
{

/**
 * A TestRunner keeps unit test stats and has test utilties.
 *
 * @author David I. Lehn
 */
class TestRunner
{
public:
   /**
    * The output level.
    */
   enum OutputLevel {
      /**
       * No output.
       */
      None = 0,
      /**
       * Final output of total tests and number passed.
       */
      Final,
      /**
       * Output "." for pass, "F" for fail, + Final.
       */
      Progress,
      /**
       * Name of test and "... PASS." or "... FAIL." + Final.
       */
      Names,
      /**
       * Names + test timing.
       */
      Times
   };

protected:
   /**
    * App running this test.
    */
   db::app::App* mApp;

   /**
    * Total number of started tests.
    */
   unsigned int mTotal;

   /**
    * Total number of passing tests.
    */
   unsigned int mPassed;

   /**
    * Total number of failed tests.
    */
   unsigned int mFailed;

   /**
    * Total number of warnings.
    */
   unsigned int mWarnings;

   /**
    * Total number of passing tests.
    */
   OutputLevel mOutputLevel;

   /**
    * If exception caught then call done() and fail via assert.
    */
   bool mDoneOnException;

   /**
    * Stack of current tests.
    */
   std::vector<std::string> mTestPath;

   /**
    * Get a test name based on current mTests stack.
    *
    * @returns test name
    */
   virtual std::string getTestName();

public:
   /**
    * Creates a new TestRunner.
    *
    * @param app app running the tests.
    * @param doneOnException if exception caught then call done() and
    *                        fail via assert.
    * @param outputLevel OutputLevel to use.
    */
   TestRunner(
      db::app::App* app, bool doneOnException, OutputLevel outputLevel = Names);

   /**
    * Destructs this TestRunner.
    */
   virtual ~TestRunner();

   /**
    * Get the app for this TestRunner.
    *
    * @return the app for this TestRunner.
    */
   virtual db::app::App* getApp();

   /**
    * Get the output level.
    *
    * @return the output level.
    */
   virtual OutputLevel getOutputLevel();

   /**
    * Case insensitive conversion from string to OuputLevel.
    *
    * @param slevel the string to convert.
    * @param level the output level.
    *
    * @return true if found and level will be set, false if not found.
    */
   static bool stringToLevel(const char *slevel, OutputLevel& level);

   /**
    * Conversion from OutputLevel to string
    *
    * @param type the OutputLevel to convert.
    *
    * @return the string or NULL.
    */
   static const char* levelToString(OutputLevel level);

   /**
    * Start a test group.  Pushes name on a stack and can be called recursively.
    * Must be paried with ungroup().
    *
    * @param name simple description of this test
    */
   virtual void group(const char* name);

   /**
    * Stop a test group.  Pops name off a stack.
    */
   virtual void ungroup();

   /**
    * Start a test. Must be paried with pass() or fail().
    *
    * @param name simple description of this test
    */
   virtual void test(const char* name);

   /**
    * Marks most recent test as passing.
    */
   virtual void pass();

   /**
    * Pass most recent test if no exception.
    */
   virtual void passIfNoException();

   /**
    * Pass most recent test if exception set.
    *
    * @param dump call dumpException() if true
    */
   virtual void passIfException(bool dump = false);

   /**
    * Marks most recent test as failed.
    *
    * @param reason description of the test failure
    */
   virtual void fail(const char* reason = NULL);

   /**
    * Notes a warning.  Not paired with a test() call or included in totals.
    * Can be used to note test that need to be written.  Best if used outside
    * a test so output is not mixed in between a test name and result.
    *
    * @param reason description of the warning
    */
   virtual void warning(const char* reason = NULL);

   /**
    * Print out final report.
    */
   virtual void done();
};

} // end namespace test
} // end namespace db

#endif
