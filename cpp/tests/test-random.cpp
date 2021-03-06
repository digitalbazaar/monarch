/*
 * Copyright (c) 2009-2010 Digital Bazaar, Inc. All rights reserved.
 */
#define __STDC_FORMAT_MACROS

#include "monarch/util/Random.h"
#include "monarch/util/Math.h"
#include "monarch/rt/Thread.h"

#include <stdlib.h>
#include <stdio.h>

using namespace monarch::rt;
using namespace monarch::util;

class RandomRunnable : public Runnable
{
public:
   RandomRunnable() {}
   virtual ~RandomRunnable() {}
   virtual void run()
   {
      int count = 20;
      printf("Generating %i random numbers between 1 and 1000000000:\n", count);

      for(int i = 1; i <= count; ++i)
      {
         printf("%i: %" PRIu64 "\n", i, Random::next(1, 1000000000));
      }

      printf("Done. Total:1 Passed:1 Failed:0 Warnings:0 Unknown:0.");
   }
};

int main()
{
   printf("Testing Random...\n");

   Random::seed();
   for(int i = 0; i < 10; ++i)
   {
      RandomRunnable r;
      Thread t(&r);
      t.start();
      t.join();
   }

   exit(0);
}
