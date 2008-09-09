/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/modest/OperationList.h"

#include <algorithm>

using namespace std;
using namespace db::modest;
using namespace db::rt;

OperationList::OperationList()
{
}

OperationList::~OperationList()
{
   terminate();
}

Operation& OperationList::operator[](int index)
{
   Operation* op = NULL;
   
   mLock.lock();
   {
      // assumes index is not invalid
      list<Operation>::iterator i = mOperations.begin();
      for(int count = 0; count < index; i++, count++);
      op = &(*i);
   }
   mLock.unlock();
   
   return *op;
}

void OperationList::add(Operation& op)
{
   mLock.lock();
   {
      mOperations.push_back(op);
   }
   mLock.unlock();
}

void OperationList::remove(Operation& op)
{
   mLock.lock();
   {
      list<Operation>::iterator i =
         find(mOperations.begin(), mOperations.end(), op);
      if(i != mOperations.end())
      {
         mOperations.erase(i);
      }
   }
   mLock.unlock();
}

void OperationList::queue(OperationRunner* opRunner)
{
   mLock.lock();
   {
      for(list<Operation>::iterator i = mOperations.begin();
          i != mOperations.end(); i++)
      {
         opRunner->runOperation(*i);
      }
   }
   mLock.unlock();
}

void OperationList::interrupt()
{
   mLock.lock();
   {
      for(list<Operation>::iterator i = mOperations.begin();
          i != mOperations.end(); i++)
      {
         (*i)->interrupt();
      }
   }
   mLock.unlock();
}

bool OperationList::waitFor(bool interruptible)
{
   bool rval = true;
   
   mLock.lock();
   {
      for(list<Operation>::iterator i = mOperations.begin();
          i != mOperations.end(); i++)
      {
         rval = (*i)->waitFor(interruptible);
         
         // break out if interruptible and interrupted
         if(interruptible && !rval)
         {
            break;
         }
      }
   }
   mLock.unlock();
   
   return rval;
}

void OperationList::prune()
{
   mLock.lock();
   {
      for(list<Operation>::iterator i = mOperations.begin();
          i != mOperations.end();)
      {
         if((*i)->stopped())
         {
            // remove operation from list
            i = mOperations.erase(i);
         }
         else
         {
            i++;
         }
      }
   }
   mLock.unlock();
}

void OperationList::terminate()
{
   mLock.lock();
   {
      interrupt();
      waitFor(false);
      prune();
   }
   mLock.unlock();
}

inline bool OperationList::isEmpty()
{
   return mOperations.empty();
}

inline void OperationList::clear()
{
   mOperations.clear();
}

inline int OperationList::length()
{
   return mOperations.size();
}
