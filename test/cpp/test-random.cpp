/*
 * Copyright (c) 2009 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/util/Random.h"
#include "db/util/Math.h"
#include "db/rt/Thread.h"

#include <stdlib.h>
#include <stdio.h>

using namespace db::rt;
using namespace db::util;

class RandomRunnable : public Runnable
{
public:
   RandomRunnable() {}
   virtual ~RandomRunnable() {}
   virtual void run()
   {
      int count = 20;
      printf("Generating %i random numbers between 1 and 1000000000:\n", count);
      
      for(int i = 1; i <= count; i++)
      {
         printf("%i: %llu\n", i, Random::next(1, 1000000000));
      }
      
      printf("done.\n");
   }
};

int main()
{
   printf("Testing Random...\n");
   
   Random::seed();
   for(int i = 0; i < 10; i++)
   {
      RandomRunnable r;
      Thread t(&r);
      t.start();
      t.join();
   }
   
   exit(0);
}
