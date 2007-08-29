/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/modest/OperationList.h"

using namespace std;
using namespace db::modest;

OperationList::OperationList(bool cleanup)
{
   mCleanup = cleanup;
}

OperationList::~OperationList()
{
   terminate();
}

void OperationList::add(Operation* op)
{
   lock();
   {
      mOperations.push_back(op);
   }
   unlock();
}

void OperationList::remove(Operation* op)
{
   lock();
   {
      list<Operation*>::iterator i =
         find(mOperations.begin(), mOperations.end(), op);
      if(i != mOperations.end())
      {
         mOperations.erase(i);
      }
   }
   unlock();
}

void OperationList::interrupt()
{
   lock();
   {
      for(list<Operation*>::iterator i = mOperations.begin();
          i != mOperations.end(); i++)
      {
         (*i)->interrupt();
      }
   }
   unlock();
}

void OperationList::waitFor(bool interruptible)
{
   lock();
   {
      for(list<Operation*>::iterator i = mOperations.begin();
          i != mOperations.end(); i++)
      {
         (*i)->waitFor(interruptible);
      }
   }
   unlock();
}

void OperationList::prune()
{
   lock();
   {
      for(list<Operation*>::iterator i = mOperations.begin();
          i != mOperations.end();)
      {
         if((*i)->stopped())
         {
            if(mCleanup)
            {
               // reclaim memory if clean up flag is set
               delete *i;
            }
            
            // remove operation from list
            i = mOperations.erase(i);
         }
         else
         {
            i++;
         }
      }
   }
   unlock();
}

void OperationList::terminate()
{
   lock();
   {
      interrupt();
      waitFor(false);
      prune();
   }
   unlock();
}
