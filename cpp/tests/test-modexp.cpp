/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/test/Test.h"
#include "monarch/test/Tester.h"
#include "monarch/test/TestRunner.h"
#include "monarch/crypto/BigInteger.h"
#include "monarch/util/Timer.h"

using namespace std;
using namespace monarch::test;
using namespace monarch::crypto;
using namespace monarch::io;
using namespace monarch::rt;
using namespace monarch::util;

void runModExpTest(TestRunner& tr)
{
   tr.test("modexp");

   Timer timer;
   uint64_t totalTime = 0;
   for(int i = 0; i < 100; i++)
   {
      // create 160-bit base
      BigInteger b = BigInteger::pseudoRandom(160, 0, false);
      //printf("\nbase size: %d-bit\n", b.getNumBytes() * 8);
      //printf("\nb: %s\n", b.toHex().c_str());

      // (1024-bit exponent + 1024-bit modulus = 2048-bit RSA private key)

      // create 1024-bit exponent
      BigInteger e = BigInteger::pseudoRandom(1024, 0, true);
      //printf("exponent size: %d-bit\n", e.getNumBytes() * 8);
      //printf("\ne: %s\n", e.toHex().c_str());

      // create 1024-bit modulus
      BigInteger m = BigInteger::pseudoRandom(1024, 0, true);
      //printf("modulus size: %d-bit\n", m.getNumBytes() * 8);
      //printf("\nm: %s\n", m.toHex().c_str());

      // get modexp remainder
      timer.start();
      BigInteger r = b.modexp(e, m);
      totalTime += timer.getElapsedMilliseconds();
      //printf("remainder: %s, %llu ms\n",
      //   r.toString().c_str(), timer.getElapsedMilliseconds());
   }

   double avg = totalTime / 100;
   printf("avg modexp time: %.2f ms... ", avg);

   tr.passIfNoException();
}

class DbModExpTester : public monarch::test::Tester
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

monarch::test::Tester* getDbModExpTester() { return new DbModExpTester(); }


DB_TEST_MAIN(DbModExpTester)
