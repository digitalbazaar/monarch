/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/modest/OperationList.h"

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
   
   lock();
   {
      // assumes index is not invalid
      list<Operation>::iterator i = mOperations.begin();
      for(int count = 0; count < index; i++, count++);
      op = &(*i);
   }
   unlock();
   
   return *op;
}

void OperationList::add(Operation& op)
{
   lock();
   {
      mOperations.push_back(op);
   }
   unlock();
}

void OperationList::remove(Operation& op)
{
   lock();
   {
      list<Operation>::iterator i =
         find(mOperations.begin(), mOperations.end(), op);
      if(i != mOperations.end())
      {
         mOperations.erase(i);
      }
   }
   unlock();
}

void OperationList::queue(OperationRunner* opRunner)
{
   lock();
   {
      for(list<Operation>::iterator i = mOperations.begin();
          i != mOperations.end(); i++)
      {
         opRunner->runOperation(*i);
      }
   }
   unlock();
}

void OperationList::interrupt()
{
   lock();
   {
      for(list<Operation>::iterator i = mOperations.begin();
          i != mOperations.end(); i++)
      {
         (*i)->interrupt();
      }
   }
   unlock();
}

bool OperationList::waitFor(bool interruptible)
{
   bool rval = true;
   
   lock();
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
   unlock();
   
   return rval;
}

void OperationList::prune()
{
   lock();
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

bool OperationList::isEmpty()
{
   return mOperations.empty();
}

void OperationList::clear()
{
   mOperations.clear();
}

bool OperationList::length()
{
   return mOperations.size();
}
