/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_test_Tester_H
#define db_test_Tester_H

#include <list>

#include "db/rt/Runnable.h"
#include "db/test/TestRunner.h"
#include "db/rt/DynamicObject.h"
#include "db/rt/DynamicObjectIterator.h"
#include "db/app/App.h"

namespace db
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
 * #ifdef DB_TEST_BUILD_MAIN
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
 * DB_TEST_MAIN(DbIoTester)
 * 
 * Author: David I. Lehn
 */
class Tester : public db::app::App
{
protected:
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
    * Get a specification of the command line paramters.
    * 
    * @return the command line specs
    */
   virtual db::rt::DynamicObject getCommandLineSpecs();
   
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
   virtual void setup(db::test::TestRunner& tr);

   /**
    * Teardown after running tests.
    */
   virtual void teardown(db::test::TestRunner& tr);

   /**
    * Add a Tester.
    */
   virtual void addTester(db::test::Tester* tester);

   /**
    * Run automatic unit tests.
    * 
    * @return exit status. 0 for success.
    */
   virtual int runAutomaticTests(db::test::TestRunner& tr);

   /**
    * Runs interactive unit tests.
    * 
    * @return exit status. 0 for success.
    */
   virtual int runInteractiveTests(db::test::TestRunner& tr);

   /**
    * Run testers and call runAutomaticTests() and runInteractiveTests().
    * 
    * @return exit status. 0 for success.
    */
   virtual int runTests(db::test::TestRunner& tr);
   
   /**
    * Run all tests and set exit status.
    * 
    * @return true on success, false and exception set on failure.
    */
   virtual bool runApp();
};

/**
 * Macro to ease defining and starting a Tester.
 */
#define DB_TEST_MAIN(testClassName) DB_DELEGATE_MAIN(testClassName)
} // end namespace test
} // end namespace db

#endif
