/*
 * Copyright (c) 2007-2010 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/modest/OperationList.h"

#include <algorithm>
#include "monarch/rt/ListIterator.h"

using namespace std;
using namespace monarch::modest;
using namespace monarch::rt;

OperationList::OperationList(bool locking) :
   mLocking(locking)
{
}

OperationList::~OperationList()
{
   OperationList::terminate();
}

Operation& OperationList::operator[](int index)
{
   Operation* op = NULL;

   if(mLocking)
   {
      mLock.lock();
   }

   // assumes index is not invalid
   OpList::iterator i = mOperations.begin();
   for(int count = 0; count < index; ++i, ++count);
   op = &(*i);

   if(mLocking)
   {
      mLock.unlock();
   }

   return *op;
}

void OperationList::add(Operation& op)
{
   if(mLocking)
   {
      mLock.lock();
   }

   mOperations.push_back(op);
   mOpIndex[&(*op)] = --mOperations.end();

   if(mLocking)
   {
      mLock.unlock();
   }
}

void OperationList::remove(Operation& op)
{
   if(mLocking)
   {
      mLock.lock();
   }

   OpIndex::iterator i = mOpIndex.find(&(*op));
   if(i != mOpIndex.end())
   {
      // remove from list and index
      mOperations.erase(i->second);
      mOpIndex.erase(i);
   }

   if(mLocking)
   {
      mLock.unlock();
   }
}

void OperationList::queue(OperationRunner* opRunner)
{
   if(mLocking)
   {
      mLock.lock();
   }

   OpList::iterator end = mOperations.end();
   for(OpList::iterator i = mOperations.begin(); i != end; ++i)
   {
      opRunner->runOperation(*i);
   }

   if(mLocking)
   {
      mLock.unlock();
   }
}

void OperationList::interrupt()
{
   if(mLocking)
   {
      mLock.lock();
   }

   OpList::iterator end = mOperations.end();
   for(OpList::iterator i = mOperations.begin(); i != end; ++i)
   {
      (*i)->interrupt();
   }

   if(mLocking)
   {
      mLock.unlock();
   }
}

bool OperationList::waitFor(bool interruptible)
{
   bool rval = true;

   if(mLocking)
   {
      mLock.lock();
   }

   OpList::iterator end = mOperations.end();
   for(OpList::iterator i = mOperations.begin(); i != end; ++i)
   {
      rval = (*i)->waitFor(interruptible);

      // break out if interruptible and interrupted
      if(interruptible && !rval)
      {
         break;
      }
   }

   if(mLocking)
   {
      mLock.unlock();
   }

   return rval;
}

void OperationList::prune()
{
   if(mLocking)
   {
      mLock.lock();
   }

   OpList::iterator end = mOperations.end();
   for(OpList::iterator i = mOperations.begin(); i != end;)
   {
      if((*i)->stopped())
      {
         // remove operation from index and list
         mOpIndex.erase(&(*(*i)));
         i = mOperations.erase(i);
      }
      else
      {
         ++i;
      }
   }

   if(mLocking)
   {
      mLock.unlock();
   }
}

void OperationList::terminate()
{
   if(mLocking)
   {
      mLock.lock();
   }

   interrupt();
   waitFor(false);
   prune();

   if(mLocking)
   {
      mLock.unlock();
   }
}

inline bool OperationList::isEmpty()
{
   return mOperations.empty();
}

inline void OperationList::clear()
{
   if(mLocking)
   {
      mLock.lock();
   }

   mOpIndex.clear();
   mOperations.clear();

   if(mLocking)
   {
      mLock.unlock();
   }
}

inline int OperationList::length()
{
   return mOperations.size();
}

IteratorRef<Operation> OperationList::getIterator()
{
   return new ListIterator<Operation>(mOperations);
}
