/*
 * Copyright (c) 2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "db/test/Test.h"
#include "db/test/Tester.h"
#include "db/test/TestRunner.h"
#include "db/rt/HashTable.h"
#include "db/rt/Runnable.h"
#include "db/rt/RunnableDelegate.h"
#include "db/rt/Thread.h"
#include "db/util/Timer.h"

using namespace std;
using namespace db::test;
using namespace db::rt;

struct KeyAsHash
{
   int operator()(int key) const
   {
      return key;
   }
};

void runHashTableTests(TestRunner& tr)
{
   tr.group("HashTable");

   HashTable<int, int, KeyAsHash> table;
   int key = 1;
   int value = 7;

   tr.test("put");
   {
      table.put(key, value);
   }
   tr.passIfNoException();

   tr.test("get");
   {
      int num;
      if(table.get(key, num))
      {
         printf("GOT: %d => %d\n", key, num);
      }
      else
      {
         printf("VALUE FOR %d NOT FOUND\n", key);
      }
   }
   tr.passIfNoException();

   tr.ungroup();
}

void runHashTableConcurrencyTest(TestRunner& tr)
{
   tr.group("HashTable concurrency");

   tr.test("single thread");
   {
   }
   tr.passIfNoException();

   tr.test("many threads");
   {
   }
   tr.passIfNoException();

   tr.ungroup();
}

class DbHashTableTester : public db::test::Tester
{
public:
   DbHashTableTester()
   {
      setName("HashTable");
   }

   /**
    * Run automatic unit tests.
    */
   virtual int runAutomaticTests(TestRunner& tr)
   {
      runHashTableTests(tr);
      return 0;
   }

   /**
    * Runs interactive unit tests.
    */
   virtual int runInteractiveTests(TestRunner& tr)
   {
      runHashTableConcurrencyTest(tr);
      return 0;
   }
};

db::test::Tester* getDbHashTableTester() { return new DbHashTableTester(); }


DB_TEST_MAIN(DbHashTableTester)
