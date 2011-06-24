/*
 * Copyright (c) 2007-2010 Digital Bazaar, Inc. All rights reserved.
 */

#include "monarch/data/json/JsonWriter.h"
#include "monarch/test/Test.h"
#include "monarch/test/TestModule.h"
#include "monarch/rt/Runnable.h"
#include "monarch/rt/System.h"
#include "monarch/rt/RunnableDelegate.h"
#include "monarch/rt/Thread.h"
#include "monarch/util/StringTools.h"

#include <cstdio>

using namespace std;
using namespace monarch::config;
using namespace monarch::data::json;
using namespace monarch::test;
using namespace monarch::rt;
using namespace monarch::util;

namespace mo_test_dyno_perf
{

static bool header = true;

static void runDynoIterTest1(
   TestRunner& tr, const char* name, int dynos, int iter)
{
   tr.test(name);
   {
      uint64_t start_init = System::getCurrentMilliseconds();
      DynamicObject d1;
      d1->setType(Array);
      for(int i = 0; i < dynos; ++i)
      {
         d1->append(i);
         //d1[i] = i;
      }
      uint64_t start_iter = System::getCurrentMilliseconds();
      for(int j = 0; j < iter; ++j)
      {
         DynamicObjectIterator i = d1.getIterator();
         while(i->hasNext())
         {
            i->next();
         }
      }
      uint64_t iter_dt = System::getCurrentMilliseconds() - start_iter;
      uint64_t init_dt = start_iter - start_init;

      if(header)
      {
         printf(
            "%9s %9s "
            "%8s %9s "
            "%8s %10s %9s "
            "%9s\n",
            "dynos", "iter",
            "init (s)", "d/ms",
            "iter (s)", "i/s", "(d*i)/ms",
            "total (s)");
         header = false;
      }
      printf(
         "%9d %9d "
         "%8.3f %9.3f "
         "%8.3f %10.3f %9.3f "
         "%9.3f\n",
         dynos, iter,
         init_dt/1000.0, dynos/(double)init_dt,
         iter_dt/1000.0, iter/(iter_dt/1000.0), (dynos*iter)/(double)iter_dt,
         (init_dt + iter_dt)/1000.0);
   }
   tr.passIfNoException();
}

static void runDynoIterTest(TestRunner& tr)
{
   tr.group("DynamicObject iter perf");

   bool all = false;
   Config cfg = tr.getApp()->getConfig();
   if(cfg->hasMember("all"))
   {
      all = cfg["all"]->getBoolean();
   }
   if(all)
   {
      //runDynoIterTest1(tr, "array s:10M  i:1    ", 10000000, 1);
      runDynoIterTest1(tr, "array s:1M   i:1    ", 1000000,  1);
      runDynoIterTest1(tr, "array s:1M   i:2    ", 1000000,  2);
      runDynoIterTest1(tr, "array s:1M   i:5    ", 1000000,  5);
      runDynoIterTest1(tr, "array s:1M   i:10   ", 1000000, 10);
   }
   runDynoIterTest1(tr, "array s:100K i:100  ", 100000, 100);
   runDynoIterTest1(tr, "array s:10K  i:1K   ", 10000, 1000);
   runDynoIterTest1(tr, "array s:1K   i:10K  ", 1000, 10000);
   runDynoIterTest1(tr, "array s:100  i:100K ", 100, 100000);
   runDynoIterTest1(tr, "array s:10   i:1M   ", 10, 1000000);
   if(all)
   {
      runDynoIterTest1(tr, "array s:5    i:1M   ", 5,  1000000);
      runDynoIterTest1(tr, "array s:2    i:1M   ", 2,  1000000);
      runDynoIterTest1(tr, "array s:1    i:1M   ", 1,  1000000);
      runDynoIterTest1(tr, "array s:0    i:1M   ", 0,  1000000);
      //runDynoIterTest1(tr, "array s:5    i:2M   ", 5,  2000000);
      //runDynoIterTest1(tr, "array s:2    i:5M   ", 2,  5000000);
      //runDynoIterTest1(tr, "array s:1    i:10M  ", 1, 10000000);
   }

   tr.ungroup();
}

static void _createDeepObject(DynamicObject& obj, int depth, int width)
{
   if(depth == 0)
   {
      // leaf, fill with random #
      obj = (uint32_t)random();
   }
   else
   {
      // make branches
      for(int i = 0; i < width; i++)
      {
         string key = StringTools::format("%d-%d", depth, i);
         DynamicObject& node = obj[key.c_str()];
         _createDeepObject(node, depth - 1, width);
      }
   }
}

// pass in obj with 'clones' and 'object' to clone
static void _runDynoCloneStressTest(DynamicObject& data)
{
   int id = data["id"]->getUInt32();
   int clones = data["clones"]->getUInt32();
   DynamicObject& object = data["object"];

   printf("Thread %d starting. %d clones.\n", id, clones);
   uint64_t start = System::getCurrentMilliseconds();
   uint64_t stat_start = System::getCurrentMilliseconds();

   for(int i = 0; i < clones; i++)
   {
      DynamicObject clone = object.clone();
      // print stats every 30s or so
      uint64_t now = System::getCurrentMilliseconds();
      uint64_t stat_dt = now - stat_start;
      if(stat_dt > 5000)
      {
         uint64_t dt = System::getCurrentMilliseconds() - start;
         printf("Thread %d @ %.3f%%. c/s=%.3f\n",
            id, ((float)i*100)/clones, (dt == 0) ? 0.0f : i/(dt/1000.0f));
         stat_start = now;
      }
   }

   uint64_t dt = System::getCurrentMilliseconds() - start;
   printf("Thread %d done. %d clones. dt=%.3f c/s=%.3f\n",
      id, clones, dt/1000.0f, (dt == 0) ? 0.0f : clones/(dt/1000.0f));
};

static void runDynoCloneStressTest(
   TestRunner& tr)
{
   Config cfg = tr.getApp()->getConfig();
   // thread count
   int threads = cfg->hasMember("threads") ? cfg["threads"]->getInt32() : 1;
   // clones to perform in each thread
   int clones = cfg->hasMember("clones") ? cfg["clones"]->getInt32() : 1;
   // depth of tree to clone
   int depth = cfg->hasMember("depth") ? cfg["depth"]->getInt32() : 1;
   // number of elements in each node of the tree
   int width = cfg->hasMember("width") ? cfg["width"]->getInt32() : 1;
   // print dyno stats
   bool stats = cfg->hasMember("stats") ? cfg["stats"]->getBoolean() : false;

   // create the config and object to clone
   DynamicObject object;
   _createDeepObject(object, depth, width);
   //JsonWriter::writeToStdOut(obj);

   Thread* threadgroup[threads];
   DynamicObject data;

   for(int ti = 0; ti < threads; ++ti)
   {
      DynamicObject d = data[ti];
      d["id"] = ti;
      d["clones"] = clones;
      d["object"] = object;
      RunnableRef r = new RunnableDelegate<void>(_runDynoCloneStressTest, d);
      threadgroup[ti] = new Thread(r);
   }
   uint64_t start = System::getCurrentMilliseconds();
   for(int ti = 0; ti < threads; ++ti)
   {
      threadgroup[ti]->start();
   }
   for(int ti = 0; ti < threads; ++ti)
   {
      threadgroup[ti]->join();
      delete threadgroup[ti];
   }
   uint64_t dt = System::getCurrentMilliseconds() - start;
   uint32_t totalclones = threads * clones;
   printf("All done. %d clones. dt=%.3f c/s=%.3f\n",
      totalclones, dt/1000.0f, (dt == 0) ? 0.0f : totalclones/(dt/1000.0f));

   if(stats)
   {
      JsonWriter::writeToStdOut(DynamicObjectImpl::getStats(), false, true);
   }
}

static bool run(TestRunner& tr)
{
   if(tr.isTestEnabled("dyno-iter-perf"))
   {
      Config cfg = tr.getApp()->getConfig();
      // number of loops for each test
      int loops = cfg->hasMember("loops") ? cfg["loops"]->getInt32() : 1;
      for(int i = 0; i < loops; ++i)
      {
         runDynoIterTest(tr);
      }
   }

   if(tr.isTestEnabled("dyno-clone-stress"))
   {
      runDynoCloneStressTest(tr);
   }
/*
   if(tr.isTestEnabled("json-ld-context-stress"))
   {
      runJsonLdContextStressTest(tr);
   }
*/
   return true;
}

} // end namespace

MO_TEST_MODULE_FN("monarch.tests.dyno-perf.test", "1.0", mo_test_dyno_perf::run)
