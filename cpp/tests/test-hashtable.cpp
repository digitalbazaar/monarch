/*
 * Copyright (c) 2009-2010 Digital Bazaar, Inc. All rights reserved.
 */
#define __STDC_CONSTANT_MACROS
#define __STDC_FORMAT_MACROS

#include "monarch/test/Test.h"
#include "monarch/test/TestModule.h"
#include "monarch/rt/ExclusiveLock.h"
#include "monarch/rt/HashTable.h"
#include "monarch/rt/Runnable.h"
#include "monarch/rt/RunnableDelegate.h"
#include "monarch/rt/SharedLock.h"
#include "monarch/rt/Thread.h"
#include "monarch/util/Timer.h"

#include <cmath>
#include <cstdio>

using namespace std;
using namespace monarch::config;
using namespace monarch::rt;
using namespace monarch::test;
using namespace monarch::util;

namespace mo_test_hashtable
{

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
      return ((uint64_t)key) & 0xFFFFFFFF;
   }
};

struct JavaHashCodeAsHash
{
   int operator()(const char* key) const
   {
      // s[0]*31^(n-1) + s[1]*31^(n-2) + ... + s[n-1]
      int n = strlen(key) - 1;
      int hash = 0;
      for(const char* ptr = key; ptr[0] != '\0'; ++ptr)
      {
         hash += (int)ptr[0] * pow((double)31, n);
         --n;
      }
      return hash;
   }
};

/* Grabbed from: http://www.azillionmonkeys.com/qed/hash.html
 *
 * Look into licensing issues if we use this for more than just testing.
 */
#undef get16bits
#if (defined(__GNUC__) && defined(__i386__)) || defined(__WATCOMC__) \
  || defined(_MSC_VER) || defined (__BORLANDC__) || defined (__TURBOC__)
#define get16bits(d) (*((const uint16_t *)(d)))
#endif

#if !defined (get16bits)
#define get16bits(d) ((((uint32_t)(((const uint8_t *)(d))[1])) << 8)\
                       +(uint32_t)(((const uint8_t *)(d))[0]) )
#endif
struct SuperFastHash
{
   int operator()(int key) const
   {
      const char* data = (const char*)&key;
      int len = sizeof(int);
      int hash = len;
      int tmp;
      int rem;

      if(len <= 0 || data == NULL) return 0;

      rem = len & 3;
      len >>= 2;

      /* Main loop */
      for(; len > 0; --len)
      {
         hash += get16bits(data);
         tmp = (get16bits(data + 2) << 11) ^ hash;
         hash = (hash << 16) ^ tmp;
         data += 2 * sizeof(uint16_t);
         hash += hash >> 11;
      }

      /* Handle end cases */
      switch(rem)
      {
         case 3:
            hash += get16bits (data);
            hash ^= hash << 16;
            hash ^= data[sizeof (uint16_t)] << 18;
            hash += hash >> 11;
            break;
         case 2:
            hash += get16bits (data);
            hash ^= hash << 11;
            hash += hash >> 17;
            break;
         case 1:
            hash += *data;
            hash ^= hash << 10;
            hash += hash >> 1;
            break;
      }

      /* Force "avalanching" of final 127 bits */
      hash ^= hash << 3;
      hash += hash >> 5;
      hash ^= hash << 4;
      hash += hash >> 17;
      hash ^= hash << 25;
      hash += hash >> 6;

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

static void runHashTableTests(TestRunner& tr)
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
};

/**
 * Base stats for a Runnable that will mash on some data structures.
 */
class HashMashBase : public Runnable
{
public:
   uint32_t mLoops; // number of times to loop writes-read process
   uint32_t mSlots; // number of key slots to use
   uint32_t mWrites; // write ops
   uint32_t mReads; // read ops
   uint32_t mOps; // total ops
   uint64_t mWriteTime;
   uint64_t mReadTime;
   uint64_t mTime; // total time
   HashMashBase(
      uint32_t loops, uint32_t slots, uint32_t writes, uint32_t reads) :
      mLoops(loops),
      mSlots(slots),
      mWrites(writes),
      mReads(reads),
      mOps(loops * (writes + reads)),
      mWriteTime(0), mReadTime(0), mTime(0) {};
   virtual ~HashMashBase() {};
   virtual void run() {};
};
template<typename _K, typename _V>
class HashMash : public HashMashBase
{
protected:
   map<_K, _V>* mMap;
   HashTable<_K, _V, SuperFastHash>* mHT;
   ExclusiveLock* mExclusiveLock;
   SharedLock* mSharedLock;
public:
   HashMash(
      uint32_t loops, uint32_t slots, uint32_t writes, uint32_t reads,
      map<_K, _V>* m,
      HashTable<_K, _V, SuperFastHash>* h,
      ExclusiveLock* exclusiveLock,
      SharedLock* sharedLock) :
      HashMashBase(loops, slots, writes, reads),
      mMap(m),
      mHT(h),
      mExclusiveLock(exclusiveLock),
      mSharedLock(sharedLock) {};
   virtual ~HashMash() {};
   virtual void run()
   {
      if(mMap) mashMap();
      if(mHT) mashHT();
   };
   virtual void mashMap()
   {
      // initialize
      for(uint32_t i = 0; i < mSlots; ++i)
      {
         if(mExclusiveLock) mExclusiveLock->lock();
         if(mSharedLock) mSharedLock->lockExclusive();
         (*mMap)[i] = 0;
         if(mExclusiveLock) mExclusiveLock->unlock();
         if(mSharedLock) mSharedLock->unlockExclusive();
      }
      uint32_t slot = 0;
      for(uint32_t loop = 0; loop < mLoops; ++loop)
      {
         {
            uint64_t start = Timer::startTiming();
            for(uint32_t i = 0; i < mWrites; ++i)
            {
               if(mExclusiveLock) mExclusiveLock->lock();
               if(mSharedLock) mSharedLock->lockExclusive();
               (*mMap)[slot++ % mSlots] = i;
               if(mExclusiveLock) mExclusiveLock->unlock();
               if(mSharedLock) mSharedLock->unlockExclusive();
            }
            mWriteTime += Timer::getMilliseconds(start);
         }
         {
            uint32_t v;
            uint64_t start = Timer::startTiming();
            for(uint32_t i = 0; i < mReads; ++i)
            {
               if(mExclusiveLock) mExclusiveLock->lock();
               if(mSharedLock) mSharedLock->lockShared();
               v = (*mMap)[slot++ % mSlots];
               if(mExclusiveLock) mExclusiveLock->unlock();
               if(mSharedLock) mSharedLock->unlockShared();
            }
            mReadTime += Timer::getMilliseconds(start);
         }
         mTime = mWriteTime + mReadTime;
      }
   };
   virtual void mashHT()
   {
      // NOTE: the locks are probably going to be NULL but the code is here to
      // normalize the timing of the conditional checking with the mashMap()
      // call.
      // initialize
      for(uint32_t i = 0; i < mSlots; ++i)
      {
         if(mExclusiveLock) mExclusiveLock->lock();
         if(mSharedLock) mSharedLock->lockExclusive();
         mHT->put(i, 0);
         if(mExclusiveLock) mExclusiveLock->unlock();
         if(mSharedLock) mSharedLock->unlockExclusive();
      }
      uint32_t slot = 0;
      for(uint32_t loop = 0; loop < mLoops; ++loop)
      {
         {
            uint64_t start = Timer::startTiming();
            for(uint32_t i = 0; i < mWrites; ++i)
            {
               if(mExclusiveLock) mExclusiveLock->lock();
               if(mSharedLock) mSharedLock->lockExclusive();
               mHT->put(slot++ % mSlots, i);
               if(mExclusiveLock) mExclusiveLock->unlock();
               if(mSharedLock) mSharedLock->unlockExclusive();
            }
            mWriteTime += Timer::getMilliseconds(start);
         }
         {
            uint32_t v;
            uint64_t start = Timer::startTiming();
            for(uint32_t i = 0; i < mReads; ++i)
            {
               if(mExclusiveLock) mExclusiveLock->lock();
               if(mSharedLock) mSharedLock->lockShared();
               mHT->get(slot++ % mSlots, v);
               if(mExclusiveLock) mExclusiveLock->unlock();
               if(mSharedLock) mSharedLock->unlockShared();
            }
            mReadTime += Timer::getMilliseconds(start);
         }
         mTime = mWriteTime + mReadTime;
      }
   };
};

static void _hashMashHeader(const char* comment, const char* sep)
{
   printf(
      "%1s"
      "%8s%s%9s%s"
      "%9s%s%9s%s"
      "%9s%s%9s%s"
      "%9s%s%9s\n",
      comment,
      "wall (s)", sep, "op/ms", sep,
      "total (s)", sep, "op/ms", sep,
      "write (s)", sep, "w/ms", sep,
      "read (s)", sep, "r/ms");
};

static void _hashMashInfo(const char* comment, const char* info)
{
   printf("%s %s\n", comment, info);
};

static void _hashMashStats(
   uint32_t threads, uint32_t loops, uint32_t slots,
   uint32_t writes, uint32_t reads,
   HashMashBase* hm[],
   uint64_t wallTime,
   const char* sep)
{
   uint64_t totalTime = 0;
   uint64_t writeTime = 0;
   uint64_t readTime = 0;
   uint64_t threadOps = loops * (writes + reads);
   uint64_t totalOps = threads * threadOps;
   uint64_t writeOps = threads * loops * writes;
   uint64_t readOps = threads * loops * reads;
   for(uint32_t i = 0; i < threads; ++i)
   {
      totalTime += hm[i]->mTime;
      writeTime += hm[i]->mWriteTime;
      readTime += hm[i]->mReadTime;
   }
   printf(
      "%9.3f%s%9.0f%s"
      "%9.3f%s%9.0f%s"
      "%9.3f%s%9.0f%s"
      "%9.3f%s%9.0f\n",
      wallTime / 1000.0, sep, totalOps / (double)wallTime, sep,
      totalTime / 1000.0, sep, totalOps / (double)totalTime, sep,
      writeTime / 1000.0, sep, writeOps / (double)writeTime, sep,
      readTime / 1000.0, sep, readOps / (double)readTime);
};

static void runHashTableConcurrencyTest(
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

static void runHashTableVsMapTest(
   TestRunner& tr,
   uint32_t threads, uint32_t loops, uint32_t slots,
   uint32_t reads, uint32_t writes, uint32_t initialSize)
{
   char name[100];
   tr.group("HashTable vs Map Single Thread");

   bool csv = true;
   const char* comment = csv ? "#" : "";
   const char* sep = csv ? "," : " ";

   printf("%s"
      " threads:%" PRIu32 " loops:%" PRIu32 " slots:%" PRIu32
      " w:%" PRIu32 " r:%" PRIu32 " initSize:%" PRIu32 "\n",
      comment, threads, loops, slots, writes, reads, initialSize);
   _hashMashHeader(comment, sep);

   if(threads == 1)
   {
      snprintf(name, 100, "map RW reads:%" PRIu32 " writes:%" PRIu32,
         reads, writes);
      tr.test(name);
      {
         _hashMashInfo(comment, "map<int, uint32_t>");
         map<int, uint32_t> m;
         Thread* t[threads];
         HashMashBase* mashers[threads];
         for(uint32_t i = 0; i < threads; ++i)
         {
            mashers[i] = new HashMash<int, uint32_t>(
               loops, slots, writes, reads, &m, NULL, NULL, NULL);
            t[i] = new Thread(mashers[i]);
         }
         uint64_t start = Timer::startTiming();
         for(uint32_t i = 0; i < threads; ++i)
         {
            t[i]->start();
         }
         for(uint32_t i = 0; i < threads; ++i)
         {
            t[i]->join();
         }
         uint64_t wallTime = Timer::getMilliseconds(start);
         for(uint32_t i = 0; i < threads; ++i)
         {
            delete mashers[i];
            delete t[i];
         }
         _hashMashStats(
            threads, loops, slots, writes, reads, mashers, wallTime, sep);
      }
      tr.passIfNoException();
   }

   snprintf(name, 100, "map excl lock RW reads:%" PRIu32 " writes:%" PRIu32,
      reads, writes);
   tr.test(name);
   {
      _hashMashInfo(comment, "map<int, uint32_t> w/ ExclusiveLock");
      map<int, uint32_t> m;
      ExclusiveLock lock;
      Thread* t[threads];
      HashMashBase* mashers[threads];
      for(uint32_t i = 0; i < threads; ++i)
      {
         mashers[i] = new HashMash<int, uint32_t>(
            loops, slots, writes, reads, &m, NULL, &lock, NULL);
         t[i] = new Thread(mashers[i]);
      }
      uint64_t start = Timer::startTiming();
      for(uint32_t i = 0; i < threads; ++i)
      {
         t[i]->start();
      }
      for(uint32_t i = 0; i < threads; ++i)
      {
         t[i]->join();
      }
      uint64_t wallTime = Timer::getMilliseconds(start);
      for(uint32_t i = 0; i < threads; ++i)
      {
         delete mashers[i];
         delete t[i];
      }
      _hashMashStats(
         threads, loops, slots, writes, reads, mashers, wallTime, sep);
   }
   tr.passIfNoException();

   snprintf(name, 100, "map shared lock RW reads:%" PRIu32 " writes:%" PRIu32,
      reads, writes);
   tr.test(name);
   {
      _hashMashInfo(comment, "map<int, uint32_t> w/ SharedLock");
      map<int, uint32_t> m;
      SharedLock lock;
      Thread* t[threads];
      HashMashBase* mashers[threads];
      for(uint32_t i = 0; i < threads; ++i)
      {
         mashers[i] = new HashMash<int, uint32_t>(
            loops, slots, writes, reads, &m, NULL, NULL, &lock);
         t[i] = new Thread(mashers[i]);
      }
      uint64_t start = Timer::startTiming();
      for(uint32_t i = 0; i < threads; ++i)
      {
         t[i]->start();
      }
      for(uint32_t i = 0; i < threads; ++i)
      {
         t[i]->join();
      }
      uint64_t wallTime = Timer::getMilliseconds(start);
      for(uint32_t i = 0; i < threads; ++i)
      {
         delete mashers[i];
         delete t[i];
      }
      _hashMashStats(
         threads, loops, slots, writes, reads, mashers, wallTime, sep);
   }
   tr.passIfNoException();

   snprintf(name, 100, "ht RW reads:%" PRIu32 " writes:%" PRIu32,
      reads, writes);
   tr.test(name);
   {
      _hashMashInfo(comment, "HashTable<int, uint32_t>");
      HashTable<int, uint32_t, SuperFastHash> h(initialSize);
      Thread* t[threads];
      HashMashBase* mashers[threads];
      for(uint32_t i = 0; i < threads; ++i)
      {
         mashers[i] = new HashMash<int, uint32_t>(
            loops, slots, writes, reads, NULL, &h, NULL, NULL);
         t[i] = new Thread(mashers[i]);
      }
      uint64_t start = Timer::startTiming();
      for(uint32_t i = 0; i < threads; ++i)
      {
         t[i]->start();
      }
      for(uint32_t i = 0; i < threads; ++i)
      {
         t[i]->join();
      }
      uint64_t wallTime = Timer::getMilliseconds(start);
      for(uint32_t i = 0; i < threads; ++i)
      {
         delete mashers[i];
         delete t[i];
      }
      _hashMashStats(
         threads, loops, slots, writes, reads, mashers, wallTime, sep);
   }
   tr.passIfNoException();

   tr.ungroup();
}

/**
 * Runs interactive unit tests.
 *
 * Options:
 * --test all - run all tests
 * --test threads - test thread concurrency
 * --test map - test speed vs map (one or more threads)
 * --test mapthreads - test speed vs map (multiple threads w/ locked map)
 * --option threads <n> - number of threads
 * --option ops <n> - number of reads and writes to do
 * --option writes <n> - override ops option for number of write operations
 * --option reads <n> - override ops option for number of read operations
 * --option loops <n> - number of times to do writes-reads process
 * --option slots <n> - number of map/hashtable keys to use
 *
 * Process will be to loop doing writes, then loop doing reads.  Adjust the
 * loops, writes, and reads options to change the ratio of operations and
 * their ordering. For example:
 *   L=1, W=2, R=0 => WW
 *   L=1, W=2, R=2 => WWRR
 *   L=2, W=2, R=2 => WWRRWWRR
 *   L=4, W=1, R=1 => WRWRWRWR
 *   L=4, W=1, R=2 => WRRWRRWRRWRR
 * The slots control a basic int key ordering based on the counter.
 *   S=1 => d[0] d[0] d[0] ...
 *   S=2 => d[0] d[1] d[0] d[1] ...
 *   S=3 => d[0] d[1] d[3] d[0] ...
 *
 * A typical run uses options like:
 * $ test-hashtable -i -l 0 --test map \
 *   --option threads <threads> --option ops <ops> --option loops <loops>
 *
 * A more complex example is to have 4 threads (ie, for a quad core system)
 * alternating between 5 writes and 95 reads 1000 times where there are 10
 * keys, run this:
 * $ test-hashtable -i -l 0 --test map \
 *   --option threads 4 --option loops 1000 --option slots 10 \
 *   --option writes 5 --option reads 95
 * That performs 4 * 1000 * (5 + 95) = 400000 operations.
 *
 * Note that as you add threads or loops it multiplies the number of ops
 * that are performed. Scale your values appropriately.
 */
static int runInteractiveTests(TestRunner& tr)
{
   Config cfg = tr.getApp()->getConfig();
   bool all = tr.isTestEnabled("all");

   uint32_t threads =
      cfg->hasMember("threads") ? cfg["threads"]->getUInt32() : 1;
   uint32_t ops =
      cfg->hasMember("ops") ? cfg["ops"]->getUInt32() : 0;
   uint32_t writes =
      cfg->hasMember("writes") ? cfg["writes"]->getUInt32() : ops;
   uint32_t reads =
      cfg->hasMember("reads") ? cfg["reads"]->getUInt32() : ops;
   uint32_t loops =
      cfg->hasMember("loops") ? cfg["loops"]->getUInt32() : 1;
   uint32_t slots =
      cfg->hasMember("slots") ? cfg["slots"]->getUInt32() : 1;
   uint32_t initialSize =
      cfg->hasMember("initialSize") ? cfg["initialSize"]->getUInt32() : 10;

   if(all || tr.isTestEnabled("threads"))
   {
      runHashTableConcurrencyTest(tr, threads, reads, writes);
   }
   if(all || tr.isTestEnabled("map"))
   {
      runHashTableVsMapTest(
         tr, threads, loops, slots, reads, writes, initialSize);
   }
   return 0;
}

static bool run(TestRunner& tr)
{
   if(tr.isDefaultEnabled())
   {
      runHashTableTests(tr);
   }
   if(tr.isTestEnabled("all") ||
      tr.isTestEnabled("threads") ||
      tr.isTestEnabled("map"))
   {
      runInteractiveTests(tr);
   }
   return true;
}

} // end namespace

MO_TEST_MODULE_FN("monarch.tests.hashtable.test", "1.0", mo_test_hashtable::run)
