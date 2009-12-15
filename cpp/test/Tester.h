/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_test_Tester_H
#define monarch_test_Tester_H

#include <list>

#include "monarch/rt/Runnable.h"
#include "monarch/test/TestRunner.h"
#include "monarch/rt/DynamicObject.h"
#include "monarch/rt/DynamicObjectIterator.h"
#include "monarch/app/AppPlugin.h"

namespace monarch
{
namespace test
{

/**
 * Top-level class to make running tests easier.
 *
 * To use: subclass, fill in tests functions, and make a main() such as the
 * following.  Use the #ifdef so the cpp can be built without main() for use
 * in a combined test executable.
 *
 * #ifdef MO_TEST_BUILD_MAIN
 * int main(int argc, const char* argv[])
 * {
 *    App app;
 *    DbIoTester tester;
 *    app.setDelegate(tester);
 *    app.initialize();
 *    return app.main(argc, argv);
 * }
 * #endif
 *
 * A macro is provided to do similar:
 * MO_TEST_MAIN(DbIoTester)
 *
 * Author: David I. Lehn
 */
class Tester : public monarch::app::AppPlugin
{
protected:
   /**
    * Name of the tester.
    */
   char* mName;

   /**
    * Sub-Testers to run.
    */
   std::list<Tester*> mTesters;

public:
   /**
    * Create a Tester.
    */
   Tester();

   /**
    * Deconstruct this Tester.
    */
   virtual ~Tester();

   /**
    * Set the owner App for this and sub testers.
    *
    * @param app the App.
    */
   virtual void setApp(monarch::app::App* app);

   /**
    * Set the name.
    *
    * @param name the name.
    */
   virtual void setName(const char* name);

   /**
    * Get the name.
    *
    * @return the name.
    */
   virtual const char* getName();

   /**
    * Get a specification of the command line paramters.
    *
    * @return the command line specs
    */
   virtual monarch::rt::DynamicObject getCommandLineSpecs();

   /**
    * Setup default option values.
    *
    * @param args read-only vector of command line arguments.
    *
    * @return true on success, false on failure and exception set
    */
   virtual bool willParseCommandLine(std::vector<const char*>* args);

   /**
    * Process the command line options
    *
    * @return true on success, false on failure and exception set
    */

   virtual bool didParseCommandLine();

   /**
    * Setup before running tests.
    */
   virtual void setup(monarch::test::TestRunner& tr);

   /**
    * Teardown after running tests.
    */
   virtual void teardown(monarch::test::TestRunner& tr);

   /**
    * Add a Tester.
    */
   virtual void addTester(monarch::test::Tester* tester);

   /**
    * Run automatic unit tests.
    *
    * @return exit status. 0 for success.
    */
   virtual int runAutomaticTests(monarch::test::TestRunner& tr);

   /**
    * Runs interactive unit tests.
    *
    * @return exit status. 0 for success.
    */
   virtual int runInteractiveTests(monarch::test::TestRunner& tr);

   /**
    * Run testers and call runAutomaticTests() and runInteractiveTests().
    *
    * @return exit status. 0 for success.
    */
   virtual int runTests(monarch::test::TestRunner& tr);

   /**
    * Run all tests and set exit status.
    *
    * @return true on success, false and exception set on failure.
    */
   virtual bool run();
};

/**
 * Macro to ease defining and starting a Tester.
 */
#define MO_TEST_MAIN(testClassName) MO_APP_PLUGIN_MAIN(testClassName)

} // end namespace test
} // end namespace monarch

#endif
