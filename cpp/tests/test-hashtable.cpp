/*
 * Copyright (c) 2009 Digital Bazaar, Inc. All rights reserved.
 */
#define __STDC_CONSTANT_MACROS
#define __STDC_FORMAT_MACROS

#include "db/test/Test.h"
#include "db/test/Tester.h"
#include "db/test/TestRunner.h"
#include "db/rt/ExclusiveLock.h"
#include "db/rt/HashTable.h"
#include "db/rt/Runnable.h"
#include "db/rt/RunnableDelegate.h"
#include "db/rt/SharedLock.h"
#include "db/rt/Thread.h"
#include "db/util/Timer.h"

#include <cmath>
#include <cstdio>

using namespace std;
using namespace db::config;
using namespace db::rt;
using namespace db::test;
using namespace db::util;

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
         hash += (int)ptr[0] * pow((double)31, n);
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

void runHashTableConcurrencyTest(
   TestRunner& tr, uint32_t threads, uint32_t reads, uint32_t writes)
{
   tr.group("HashTable concurrency");

   char name[100];
   snprintf(name, 100,
      "RW threads:%" PRIu32
      " reads:%" PRIu32
      " writes:%" PRIu32,
      threads, reads, writes);
   tr.test(name);
   {
   }
   tr.passIfNoException();

   tr.ungroup();
}

void runHashTableVsMapTest(
   TestRunner& tr, uint32_t reads, uint32_t writes)
{
   int threads = 1;
   char name[100];
   tr.group("HashTable vs Map Single Thread");

   bool csv = true;
   const char* comment = csv ? "#" : "";
   const char* sep = csv ? "," : " ";
   uint32_t wr = writes + reads;

   printf("%s w:%" PRIu32 " r:%" PRIu32 "\n", comment, reads, writes);
   uint64_t mrdt, mwdt;
   uint64_t hrdt, hwdt;

   snprintf(name, 100,
      "map RW"
      " reads:%" PRIu32
      " writes:%" PRIu32,
      reads, writes);
   tr.test(name);
   {
      uint64_t rstart, wstart;

      map<int, uint32_t> m;

      printf("%s map\n", comment);
      printf(
         "%1s%8s%s"
         "%9s%s%9s%s"
         "%9s%s%9s%s"
         "%9s%s%9s\n",
         comment, "threads", sep,
         "total (s)", sep, "rw/s", sep,
         "write (s)", sep, "w/s", sep,
         "read (s)", sep, "r/s");

      wstart = Timer::startTiming();
      for(uint32_t i = 0; i < writes; i++)
      {
         m[0] = i;
      }
      mwdt = Timer::getMilliseconds(wstart);

      uint32_t v;
      rstart = Timer::startTiming();
      for(uint32_t i = 0; i < reads; i++)
      {
         v = m[0];
      }
      mrdt = Timer::getMilliseconds(rstart);

      uint64_t dt = mwdt + mrdt;
      printf(
         "%9d%s"
         "%9.3f%s%9.0f%s"
         "%9.3f%s%9.0f%s"
         "%9.3f%s%9.0f\n",
         threads, sep,
         dt/1000.0, sep, wr/(double)dt, sep,
         mwdt/1000.0, sep, writes/(double)mwdt, sep,
         mrdt/1000.0, sep, reads/(double)mrdt);
   }
   tr.passIfNoException();

   snprintf(name, 100,
      "ht RW"
      " reads:%" PRIu32
      " writes:%" PRIu32,
      reads, writes);
   tr.test(name);
   {
      uint64_t rstart, wstart;

      HashTable<int, uint32_t, KeyAsHash> h;

      printf("%s ht\n", comment);
      printf(
         "%1s%8s%s"
         "%9s%s%9s%s"
         "%9s%s%9s%s"
         "%9s%s%9s\n",
         comment, "threads", sep,
         "total (s)", sep, "rw/s", sep,
         "write (s)", sep, "w/s", sep,
         "read (s)", sep, "r/s");

      wstart = Timer::startTiming();
      for(uint32_t i = 0; i < writes; i++)
      {
         h.put(0, i);
      }
      hwdt = Timer::getMilliseconds(wstart);

      uint32_t v;
      rstart = Timer::startTiming();
      for(uint32_t i = 0; i < reads; i++)
      {
         h.get(0, v);
      }
      hrdt = Timer::getMilliseconds(rstart);

      uint64_t dt = hwdt + hrdt;
      printf(
         "%9d%s"
         "%9.3f%s%9.0f%s"
         "%9.3f%s%9.0f%s"
         "%9.3f%s%9.0f\n",
         threads, sep,
         dt/1000.0, sep, wr/(double)dt, sep,
         hwdt/1000.0, sep, writes/(double)hwdt, sep,
         hrdt/1000.0, sep, reads/(double)hrdt);
   }
   tr.passIfNoException();

   printf("%s ratio ht/m\n", comment);
   printf(
      "%9d%s"
      "%9.3f%s%9s%s"
      "%9.3f%s%9s%s"
      "%9.3f%s%9s\n",
      threads, sep,
      ((double)hwdt+hrdt)/(mwdt+mrdt), sep, "", sep,
      ((double)hwdt/mwdt), sep, "", sep,
      ((double)hrdt/mrdt), sep, "");

   snprintf(name, 100,
      "map excl lock RW"
      " reads:%" PRIu32
      " writes:%" PRIu32,
      reads, writes);
   tr.test(name);
   {
      uint64_t rstart, wstart;

      map<int, uint32_t> m;
      ExclusiveLock lock;

      printf("%s map exclusive lock\n", comment);
      printf(
         "%1s%8s%s"
         "%9s%s%9s%s"
         "%9s%s%9s%s"
         "%9s%s%9s\n",
         comment, "threads", sep,
         "total (s)", sep, "rw/s", sep,
         "write (s)", sep, "w/s", sep,
         "read (s)", sep, "r/s");

      wstart = Timer::startTiming();
      for(uint32_t i = 0; i < writes; i++)
      {
         lock.lock();
         m[0] = i;
         lock.unlock();
      }
      mwdt = Timer::getMilliseconds(wstart);

      uint32_t v;
      rstart = Timer::startTiming();
      for(uint32_t i = 0; i < reads; i++)
      {
         lock.lock();
         v = m[0];
         lock.unlock();
      }
      mrdt = Timer::getMilliseconds(rstart);

      uint64_t dt = mwdt + mrdt;
      printf(
         "%9d%s"
         "%9.3f%s%9.0f%s"
         "%9.3f%s%9.0f%s"
         "%9.3f%s%9.0f\n",
         threads, sep,
         dt/1000.0, sep, wr/(double)dt, sep,
         mwdt/1000.0, sep, writes/(double)mwdt, sep,
         mrdt/1000.0, sep, reads/(double)mrdt);
   }
   tr.passIfNoException();

   snprintf(name, 100,
      "map shared lock RW"
      " reads:%" PRIu32
      " writes:%" PRIu32,
      reads, writes);
   tr.test(name);
   {
      uint64_t rstart, wstart;

      map<int, uint32_t> m;
      SharedLock lock;

      printf("%s map shared lock\n", comment);
      printf(
         "%1s%8s%s"
         "%9s%s%9s%s"
         "%9s%s%9s%s"
         "%9s%s%9s\n",
         comment, "threads", sep,
         "total (s)", sep, "rw/s", sep,
         "write (s)", sep, "w/s", sep,
         "read (s)", sep, "r/s");

      wstart = Timer::startTiming();
      for(uint32_t i = 0; i < writes; i++)
      {
         lock.lockExclusive();
         m[0] = i;
         lock.unlockExclusive();
      }
      mwdt = Timer::getMilliseconds(wstart);

      uint32_t v;
      rstart = Timer::startTiming();
      for(uint32_t i = 0; i < reads; i++)
      {
         lock.lockShared();
         v = m[0];
         lock.unlockShared();
      }
      mrdt = Timer::getMilliseconds(rstart);

      uint64_t dt = mwdt + mrdt;
      printf(
         "%9d%s"
         "%9.3f%s%9.0f%s"
         "%9.3f%s%9.0f%s"
         "%9.3f%s%9.0f\n",
         threads, sep,
         dt/1000.0, sep, wr/(double)dt, sep,
         mwdt/1000.0, sep, writes/(double)mwdt, sep,
         mrdt/1000.0, sep, reads/(double)mrdt);
   }
   tr.passIfNoException();

   tr.ungroup();
}

void runHashTableVsMapThreadsTest(
   TestRunner& tr, uint32_t threads, uint32_t reads, uint32_t writes)
{
   tr.group("HashTable vs Map Multiple Threads");

   char name[100];
   snprintf(name, 100,
      "RW threads:%" PRIu32
      " reads:%" PRIu32
      " writes:%" PRIu32,
      threads, reads, writes);
   tr.test(name);
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
    *
    * Options:
    * --test all - run all tests
    * --test threads - test thread concurrency
    * --test map - test speed vs map (single threaded)
    * --test mapthreads - test speed vs map (multiple threads w/ locked map)
    * --option threads <n> - number of threads
    * --option reads <n> - number of read operations
    * --option writes <n> - number of write operations
    */
   virtual int runInteractiveTests(TestRunner& tr)
   {
      Config cfg = tr.getApp()->getConfig();
      const char* test = cfg["db.test.Tester"]["test"]->getString();
      bool all = (strcmp(test, "all") == 0);

      uint32_t threads =
         cfg->hasMember("threads") ? cfg["threads"]->getUInt32() : 1;
      uint32_t reads =
         cfg->hasMember("reads") ? cfg["reads"]->getUInt32() : 1;
      uint32_t writes =
         cfg->hasMember("writes") ? cfg["writes"]->getUInt32() : 1;
      //uint32_t loops =
      //   cfg->hasMember("loops") ? cfg["loops"]->getUInt32() : 1;
      //uint32_t ops =
      //   cfg->hasMember("ops") ? cfg["ops"]->getUInt32() : 1;

      if(all || (strcmp(test, "threads") == 0))
      {
         runHashTableConcurrencyTest(tr, threads, reads, writes);
      }
      if(all || (strcmp(test, "map") == 0))
      {
         runHashTableVsMapTest(tr, reads, writes);
      }
      if(all || (strcmp(test, "mapthreads") == 0))
      {
         runHashTableVsMapThreadsTest(tr, threads, reads, writes);
      }
      return 0;
   }
};

db::test::Tester* getDbHashTableTester() { return new DbHashTableTester(); }


DB_TEST_MAIN(DbHashTableTester)
