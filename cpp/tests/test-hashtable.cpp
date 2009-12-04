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

#include <math.h>

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

struct AddressAsHash
{
   int operator()(const char* key) const
   {
      return (int)key;
   }
};

struct JavaHashCodeAsHash
{
   int operator()(const char* key) const
   {
      // s[0]*31^(n-1) + s[1]*31^(n-2) + ... + s[n-1]
      int n = strlen(key) - 1;
      int hash = 0;
      for(const char* ptr = key; ptr[0] != '\0'; ptr++)
      {
         hash += (int)ptr[0] * pow(31, n);
         n--;
      }
      return hash;
   }
};

struct StringEquals
{
   bool operator()(const char* key1, const char* key2)
   {
      return strcmp(key1, key2) == 0;
   }
};

void runHashTableTests(TestRunner& tr)
{
   tr.group("HashTable");

   tr.test("complex");
   {
      HashTable<int, int, KeyAsHash> table(1);

      table.put(1, 7);
      table.put(2, 13);

      int num;

      assert(table.get(1, num));
      assert(num == 7);
      assert(table.get(2, num));
      assert(num == 13);
      assert(!table.put(2, 10, false));
      assert(table.put(2, 10));
      assert(table.get(2, num));
      assert(num == 10);
      assert(table.put(2, 11));
      assert(table.get(2, num));
      assert(num == 11);

      assert(!table.get(99, num));

      HashTable<int, int, KeyAsHash> table2 = table;
      num = 0;
      assert(table.get(1, num));
      assert(num == 7);
      assert(table.get(2, num));
      assert(num == 11);

      HashTable<int, int, KeyAsHash> table3;
      table3 = table;
      num = 0;
      assert(table.get(1, num));
      assert(num == 7);
      assert(table.get(2, num));
      assert(num == 11);
   }
   tr.passIfNoException();

   tr.test("static string");
   {
      HashTable<const char*, int, AddressAsHash> table;

      table.put("foo", 7);
      table.put("bar", 13);

      int num;
      assert(table.get("foo", num));
      assert(num == 7);
      assert(table.get("bar", num));
      assert(num == 13);
   }
   tr.passIfNoException();

   tr.test("dynamic string");
   {
      HashTable<const char*, int, JavaHashCodeAsHash, StringEquals> table;

      table.put("foo", 7);
      table.put("bar", 13);

      string key1 = "foo";
      string key2 = "bar";

      int num;
      assert(table.get(key1.c_str(), num));
      assert(num == 7);
      assert(table.get(key2.c_str(), num));
      assert(num == 13);
   }
   tr.passIfNoException();

   /*
   HashTable<int, int, KeyAsHash> table(1);
   int hit = 1;
   int miss = 2;
   int value = 7;

   tr.test("put");
   {
      table.put(hit, value);
   }
   tr.passIfNoException();

   tr.test("get hit");
   {
      int num;
      if(table.get(hit, num))
      {
         printf("GOT: %d => %d\n", hit, num);
      }
      else
      {
         printf("VALUE FOR %d NOT FOUND\n", hit);
      }
   }
   tr.passIfNoException();

   tr.test("get miss");
   {
      int num;
      if(table.get(miss, num))
      {
         printf("GOT: %d => %d\n", miss, num);
      }
      else
      {
         printf("VALUE FOR %d NOT FOUND\n", miss);
      }
   }
   tr.passIfNoException();
   */

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
