/*
 * Copyright (c) 2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/rt/HazardPtrList.h"

#include "monarch/rt/Atomic.h"

using namespace db::rt;

HazardPtrList::HazardPtrList() :
   mHead(NULL)
{
}

HazardPtrList::~HazardPtrList()
{
   // clean up all hazard pointers
   HazardPtr* head = const_cast<HazardPtr*>(mHead);
   while(head != NULL)
   {
      HazardPtr* tmp = head;
      head = head->next;
      delete tmp;
   }
}

HazardPtr* HazardPtrList::acquire()
{
   volatile HazardPtr* rval = NULL;

   // find a hazard pointer that is inactive
   HazardPtr* ptr = const_cast<HazardPtr*>(mHead);
   while(rval == NULL && ptr != NULL)
   {
      // try to mark an inactive pointer as active
      if(!ptr->active && Atomic::compareAndSwap(&ptr->active, false, true))
      {
         // successfully acquired
         rval = ptr;
      }
      else
      {
         // try the next pointer
         ptr = ptr->next;
      }
   }

   // no inactive pointer found, create a new one
   if(rval == NULL)
   {
      rval = new HazardPtr;
      rval->active = true;
      rval->value = NULL;

      // atomically push the pointer onto the head of the list
      volatile HazardPtr* old;
      do
      {
         old = mHead;
         rval->next = const_cast<HazardPtr*>(old);
      }
      while(!Atomic::compareAndSwap(&mHead, old, rval));
   }

   return const_cast<HazardPtr*>(rval);
}

void HazardPtrList::release(HazardPtr* ptr)
{
   ptr->value = NULL;
   ptr->active = false;
}

bool HazardPtrList::isProtected(void* addr)
{
   bool rval = false;

   // search the list for the given address
   HazardPtr* ptr = const_cast<HazardPtr*>(mHead);
   while(!rval && ptr != NULL)
   {
      if(ptr->value == addr)
      {
         // address found
         rval = true;
      }
      else
      {
         // go to next pointer
         ptr = ptr->next;
      }
   }

   return rval;
}
