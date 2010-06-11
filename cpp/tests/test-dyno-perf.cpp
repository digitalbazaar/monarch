/*
 * Copyright (c) 2007-2010 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/test/Test.h"
#include "monarch/test/TestModule.h"
#include "monarch/rt/Runnable.h"
#include "monarch/rt/System.h"

#include <cstdio>

using namespace std;
using namespace monarch::config;
using namespace monarch::test;
using namespace monarch::rt;

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
         d1->append() = i;
         //d1[i] = i;
      }
      uint64_t start_iter = System::getCurrentMilliseconds();
      for(int j = 0; j < iter; j++)
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

static bool run(TestRunner& tr)
{
   if(tr.isTestEnabled("dyno-perf"))
   {
      Config cfg = tr.getApp()->getConfig();
      // number of loops for each test
      int loops = cfg->hasMember("loops") ? cfg["loops"]->getInt32() : 1;
      for(int i = 0; i < loops; ++i)
      {
         runDynoIterTest(tr);
      }
   }
   return true;
}

} // end namespace

MO_TEST_MODULE_FN("monarch.tests.dyno-perf.test", "1.0", mo_test_dyno_perf::run)
