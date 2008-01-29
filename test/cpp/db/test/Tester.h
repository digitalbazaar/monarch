/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_test_Tester_H
#define db_test_Tester_H

#include "db/rt/Exception.h"
#include "db/rt/Object.h"
#include "db/rt/Runnable.h"
#include "db/test/TestRunner.h"
#include "db/util/DynamicObject.h"
#include "db/util/DynamicObjectIterator.h"

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
    * Setup before running tests.
    */
   virtual void setup(db::test::TestRunner& tr);

   /**
    * Teardown after running tests.
    */
   virtual void teardown(db::test::TestRunner& tr);

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
    * Runs the unit tests.
    */
   virtual void run();
   
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

#ifdef DB_TEST_NO_MAIN
#define DB_TEST_MAIN(testClassName)
#else
#define DB_TEST_MAIN(testClassName)    \
int main(int argc, const char* argv[]) \
{                                      \
   testClassName tester;               \
   tester.main(argc, argv);            \
}
#endif

} // end namespace test
} // end namespace db

#endif
