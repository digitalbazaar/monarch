/*
 * Copyright (c) 2009 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/util/Random.h"
#include "db/util/Math.h"

#include <stdlib.h>
#include <stdio.h>

using namespace db::util;

int main()
{
   printf("Testing Random...\n");
   
   int count = 20;
   printf("Generating %i random numbers between 1 and 1000000000:\n", count);
   
   Random::seed();
   for(int i = 1; i <= count; i++)
   {
      printf("%i: %llu\n", i, Random::next(1, 1000000000));
   }
   
   printf("done.\n");
   exit(0);
}
