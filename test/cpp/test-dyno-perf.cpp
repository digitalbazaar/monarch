/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include <iostream>

#include "db/test/Test.h"
#include "db/test/Tester.h"
#include "db/test/TestRunner.h"
#include "db/rt/Runnable.h"
#include "db/rt/System.h"

using namespace std;
using namespace db::test;
using namespace db::rt;

void runDynoIterTest1(TestRunner& tr, const char* name, int size, int iter)
{
   tr.test(name);
   {
      DynamicObject d1;
      for(int i = 0; i < size; i++)
      {
         d1[i] = i;
      }
   
      uint64_t start = System::getCurrentMilliseconds();
      
      for(int j = 0; j < iter; j++)
      {
         DynamicObjectIterator i = d1.getIterator();
         while(i->hasNext())
         {
            DynamicObject next = i->next();
         }
      }
      
      uint64_t stop = System::getCurrentMilliseconds();
      
      cout << "[dt:" << stop-start << "]";
   }
   tr.passIfNoException();
}

void runDynoIterTest(TestRunner& tr)
{
   tr.group("DynamicObject iter perf");

   runDynoIterTest1(tr, "array (10k * 1k)", 10000, 1000);
   runDynoIterTest1(tr, "array (1k * 10k)", 1000, 10000);
   runDynoIterTest1(tr, "array (100 * 100k)", 100, 100000);
   runDynoIterTest1(tr, "array (10 * 1M)", 10, 1000000);

   tr.ungroup();
}

class DbDynoPerfTester : public db::test::Tester
{
public:
   DbDynoPerfTester()
   {
      setName("dyno-perf");
   }

   /**
    * Run automatic unit tests.
    */
   virtual int runAutomaticTests(TestRunner& tr)
   {
      return 0;
   }

   /**
    * Runs interactive unit tests.
    */
   virtual int runInteractiveTests(TestRunner& tr)
   {
      runDynoIterTest(tr);
      return 0;
   }
};

#ifndef DB_TEST_NO_MAIN
DB_TEST_MAIN(DbDynoPerfTester)
#endif
