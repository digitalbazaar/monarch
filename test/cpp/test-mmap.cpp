/*
 * Copyright (c) 2009 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/fiber/WindowsSupport.h"

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
   
   // mmap stacks
   size_t stackSize = 16384;
   void* func1Stack = mmap(
      0, stackSize,
      PROT_READ | PROT_WRITE | PROT_EXEC,
      MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
   if(func1Stack == MAP_FAILED)
   {
      printf("failed to allocate func1 stack\n"); 
      exit(1);
   }
   printf("func1 stack allocated.\n");
   
   void* func2Stack = mmap(
      0, stackSize,
      PROT_READ | PROT_WRITE | PROT_EXEC,
      MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
   if(func2Stack == MAP_FAILED)
   {
      printf("failed to allocate func2 stack\n"); 
      exit(1);
   }
   printf("func2 stack allocated.\n");
      
   // make func1 context
   if(getcontext(&gFunc1Context) == -1)
   {
      printf("failed to make func1 context\n"); 
      exit(1);
   }
   gFunc1Context.uc_stack.ss_sp = func1Stack;
   gFunc1Context.uc_stack.ss_size = stackSize;
   gFunc1Context.uc_stack.ss_flags = 0;
   gFunc1Context.uc_link = NULL;
   makecontext(&gFunc1Context, (void (*)())func1, 1, 1);
   
   // make func2 context
   if(getcontext(&gFunc2Context) == -1)
   {
      printf("failed to make func2 context\n"); 
      exit(1);
   }
   gFunc2Context.uc_stack.ss_sp = func2Stack;
   gFunc2Context.uc_stack.ss_size = stackSize;
   gFunc2Context.uc_stack.ss_flags = 0;
   gFunc2Context.uc_link = NULL;
   makecontext(&gFunc2Context, (void (*)())func2, 1, 2);
#if 0
   printf("main swapping in func1...\n");
   if(swapcontext(&gMainContext, &gFunc1Context) == -1)
   {
      printf("failed to swap from main to func1\n");
      exit(1);
   }
#endif
   printf("main returned, de-allocating stacks...\n");
   
   if(munmap(func1Stack, stackSize) == -1)
   {
      printf("failed to de-allocate func1 stack\n"); 
      exit(1);
   }
   
   if(munmap(func2Stack, stackSize) == -1)
   {
      printf("failed to de-allocate func2 stack\n"); 
      exit(1);
   }
   
   printf("exiting.\n");
   exit(0);
}
