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
using namespace db::io;
using namespace db::rt;
using namespace db::util;

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
