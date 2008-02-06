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
 *    DbIoTester tester();
 *    return tester.main(argc, argv);
 * }
 * #endif
 * 
 * A macro is provided to do similar:
 * DB_TEST_MAIN(DbIoTester)
 * 
 * Author: David I. Lehn
 */
class Tester : public virtual db::rt::Object, public db::rt::Runnable
{
protected:
   /**
    * Name for this Tester
    */
   char* mName;
   
   /**
    * Flags controlling testing
    */
   unsigned int flags;
   
   /**
    * Run automatic tests in runAutomaticTests().
    */
   static const int DB_TEST_AUTOMATIC = 1;
   
   /**
    * Run interactive tests in runInteractiveTests().
    */
   static const int DB_TEST_INTERACTIVE = 1 << 1;

   /**
    * Exit status to use for all tests.
    */  
   int mExitStatus;

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
    * Set the tester name.
    * 
    * @param name the name.
    */
   virtual void setName(const char* name);

   /**
    * Get the tester name.
    * 
    * @return the tester name.
    */
   virtual const char* getName();

   /**
    * Setup before running tests.
    */
   virtual void setup(db::test::TestRunner& tr);

   /**
    * Teardown after running tests.
    */
   virtual void teardown(db::test::TestRunner& tr);

   /**
    * Add a tester.
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
    * Run all tests and set mExitStatus.
    */
   virtual void run();
   
   /**
    * Initialize logging.
    */
   virtual void loggingInitialize();
   
   /**
    * Cleanup logging.
    */
   virtual void loggingCleanup();
   
   /**
    * Run all tests.
    * 
    * @param argc number of command line arguments.
    * @param argv command line arguments.
    * 
    * @return exit status. 0 for success.
    */
   virtual int main(int argc, const char* argv[]);
};

/**
 * Macro to ease defining and calling Tester::main().
 * NOTE: Surround this macro with #ifndef DB_TEST_NO_MAIN ... #endif.
 */
#define DB_TEST_MAIN(testClassName)    \
int main(int argc, const char* argv[]) \
{                                      \
   int rval;                           \
   testClassName tester;               \
   rval = tester.main(argc, argv);     \
   return rval;                        \
}

} // end namespace test
} // end namespace db

#endif
