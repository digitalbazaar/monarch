/*
 * Copyright (c) 2010 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_test_Testable_h
#define monarch_test_Testable_h

#include "monarch/test/TestRunner.h"

namespace monarch
{
namespace test
{

/**
 * A class implemention the Testable interface can be used to run tests with a
 * TestRunner.
 *
 * @author David I. Lehn <dlehn@digitalbazaar.com>
 */
class Testable
{
public:
   /**
    * Create an Testable instance.
    */
   Testable() {};

   /**
    * Deconstruct this Testable instance.
    */
   virtual ~Testable() {};

   /**
    * Run tests with the given TestRunner. Note that tests may successfully
    * record failures with the TestRunner and return true. If run() returns
    * false the testing framework itself may stop running further tests.
    *
    * @param tr a TestRunner to use for testing.
    *
    * @return true if successful, false if an exception occurred.
    */
   virtual bool run(monarch::test::TestRunner& tr) = 0;
};

} // end namespace test
} // end namespace monarch

#endif
