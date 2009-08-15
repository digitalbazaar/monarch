/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "db/test/Test.h"
#include "db/test/Tester.h"
#include "db/test/TestRunner.h"
#include "db/crypto/BigInteger.h"
#include "db/util/Timer.h"

using namespace std;
using namespace db::test;
using namespace db::crypto;
using namespace db::rt;
using namespace db::util;

void runModExpTest(TestRunner& tr)
{
   tr.test("modexp");

   // create big number
   BigInteger number1 = 2;
   number1.powEquals(2048);
   printf("\nbyte size: %d\n", number1.getNumBytes());

   tr.passIfNoException();
}

class DbModExpTester : public db::test::Tester
{
public:
   DbModExpTester()
   {
      setName("modexp");
   }

   /**
    * Run automatic unit tests.
    */
   virtual int runAutomaticTests(TestRunner& tr)
   {
      runModExpTest(tr);
      return 0;
   }

   /**
    * Runs interactive unit tests.
    */
   virtual int runInteractiveTests(TestRunner& tr)
   {
      return 0;
   }
};

db::test::Tester* getDbModExpTester() { return new DbModExpTester(); }


DB_TEST_MAIN(DbModExpTester)
