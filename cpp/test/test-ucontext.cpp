/*
 * Copyright (c) 2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "db/fiber/FiberContext.h"

#include <stdio.h>
#include <stdlib.h>

static ucontext_t gMainContext;
static ucontext_t gFunc1Context;
static ucontext_t gFunc2Context;

void func1(int i)
{
   printf("func1 running with argument %i...\n", i);
   printf("func1 swapping in func2...\n");
   if(swapcontext(&gFunc1Context, &gFunc2Context) == -1)
   {
      printf("failed to swap from func1 to func2\n");
      exit(1);
   }
   printf("func1 returned, going to main...\n");
   if(setcontext(&gMainContext) == -1)
   {
      printf("failed to go from func1 to main\n");
      exit(1);
   }
}

void func2(int i)
{
   printf("func2 running with argument %i...\n", i);
   printf("func2 swapping in func1...\n");
   if(swapcontext(&gFunc2Context, &gFunc1Context) == -1)
   {
      printf("failed to swap from func2 to func1\n");
      exit(1);
   }
}

int main()
{
   printf("Testing ucontext...\n");

   char func1Stack[16384];
   char func2Stack[16384];

   // make func1 context
   if(getcontext(&gFunc1Context) == -1)
   {
      printf("failed to make func1 context\n");
      exit(1);
   }
   gFunc1Context.uc_stack.ss_sp = func1Stack;
   gFunc1Context.uc_stack.ss_size = sizeof(func1Stack);
#ifndef MACOS
   gFunc1Context.uc_stack.ss_flags = 0;
   gFunc1Context.uc_link = NULL;
#endif
   makecontext(&gFunc1Context, (void (*)())func1, 1, 1);

   // make func2 context
   if(getcontext(&gFunc2Context) == -1)
   {
      printf("failed to make func2 context\n");
      exit(1);
   }
   gFunc2Context.uc_stack.ss_sp = func2Stack;
   gFunc2Context.uc_stack.ss_size = sizeof(func2Stack);
#ifndef MACOS
   gFunc2Context.uc_stack.ss_flags = 0;
   gFunc2Context.uc_link = NULL;
#endif
   makecontext(&gFunc2Context, (void (*)())func2, 1, 2);

   printf("main swapping in func1...\n");
   if(swapcontext(&gMainContext, &gFunc1Context) == -1)
   {
      printf("failed to swap from main to func1\n");
      exit(1);
   }

   printf("main returned, exiting.\n");
   printf("Done. Total:1 Passed:1 Failed:0 Warnings:0 Unknown:0.\n");
   exit(0);
}
