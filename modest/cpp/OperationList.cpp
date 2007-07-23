/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "OperationList.h"

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
#include <iostream>
void OperationList::waitFor(bool interruptible)
{
   lock();
   {
      for(list<Operation*>::iterator i = mOperations.begin();
          i != mOperations.end(); i++)
      {
         cout << "OperationList waiting..." << endl;
         (*i)->waitFor(interruptible);
         cout << "OperationList wait complete" << endl;
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
         if((*i)->collectable())
         {
            cout << "Pruning" << endl;
            if(mCleanup)
            {
               // reclaim memory if clean up flag is set
               delete *i;
            }
            
            // remove operation from list
            i = mOperations.erase(i);
            cout << "Pruned." << endl;
         }
         else
         {
            cout << "NOT PRUNING" << endl;
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
      cout << "interrupting all" << endl;
      interrupt();
      cout << "all interrupted" << endl;
      cout << "waiting for all" << endl;
      waitFor(false);
      cout << "wait all finished" << endl;
      cout << "pruning all" << endl;
      prune();
      cout << "all pruned." << endl;
   }
   unlock();
}
