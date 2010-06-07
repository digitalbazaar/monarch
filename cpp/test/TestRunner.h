/*
 * Copyright (c) 2007-2010 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_test_TestRunner_H
#define monarch_test_TestRunner_H

#include <map>
#include <string>
#include <vector>

#include "monarch/app/App.h"
#include "monarch/kernel/MicroKernel.h"

namespace monarch
{
namespace test
{

/**
 * A TestRunner keeps unit test stats and has test utilties.
 *
 * Testing systems can set tests to run with enableTest(...). Testing code
 * should check if a test is enabled with isTestEnabled(...) before running
 * tests. Multiple tests may be enabled at once. A default mode is enabled that
 * can be checked with the TestRunner::DEFAULT value ("default") or special
 * isDefaultEnabled() method.
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

   /**
    * Default test to run.
    */
   static const char* DEFAULT;

protected:
   /**
    * App running this test.
    */
   monarch::app::App* mApp;

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
    * List tests mode. Report all tests as disabled and print their name.
    */
   bool mListTests;

   /**
    * Stack of current tests.
    */
   std::vector<std::string> mTestPath;

   /**
    * Enabled status of tests.
    */
   std::map<std::string, bool> mTests;

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
    * @param app App running the tests.
    * @param doneOnException if exception caught then call done() and
    *           fail via assert.
    * @param outputLevel OutputLevel to use.
    */
   TestRunner(
      monarch::app::App* app,
      bool doneOnException, OutputLevel outputLevel = Names);

   /**
    * Destructs this TestRunner.
    */
   virtual ~TestRunner();

   /**
    * Get the App for this TestRunner.
    *
    * @return the App for this TestRunner.
    */
   virtual monarch::app::App* getApp();

   /**
    * Get the output level.
    *
    * @return the output level.
    */
   virtual OutputLevel getOutputLevel();

   /**
    * Set the list tests mode.
    *
    * @param listTests true to only list tests, false to run tests.
    */
   virtual void setListTests(bool listTests);

   /**
    * Set the enabled status of a test.
    */
   virtual void enableTest(std::string test, bool enabled = true);

   /**
    * Get the enabled status of a test.
    *
    * @param test the test name to check.
    *
    * @return true if test is enabled.
    */
   virtual bool isTestEnabled(std::string test);

   /**
    * Get the enabled status of the special default test.
    *
    * @return true if default test is enabled.
    */
   virtual bool isDefaultEnabled();

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
} // end namespace monarch

#endif
