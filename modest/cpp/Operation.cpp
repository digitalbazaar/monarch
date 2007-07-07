/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "Operation.h"

using namespace std;
using namespace db::rt;
using namespace db::modest;

Operation::Operation(Runnable* r, Environment* e)
{
   mRunnable = r;
   mExecutionEnvironment = e;
   mInterrupted = false;
}

Operation::~Operation()
{
}

void Operation::interrupt()
{
   // synchronize
   lock();
   {
      mInterrupted = true;
      if(mThread != NULL)
      {
         mThread->interrupt();
      }
   }
   unlock();
}

bool Operation::isInterrupted()
{
   return Thread::interrupted();
}
