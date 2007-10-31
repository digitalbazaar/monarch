/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/modest/Operation.h"

using namespace db::modest;

Operation::Operation(OperationImpl* impl)
{
   if(impl != NULL)
   {
      // create reference to implementation
      mReference = new ImplReference;
      mReference->impl = impl;
      mReference->count = 1;
   }
   else
   {
      // no reference
      mReference = NULL;
   }
}

Operation::Operation(const Operation& copy)
{
   // acquire copy's reference
   acquire(copy.mReference);
}

Operation::~Operation()
{
   // release reference
   release();
}

void Operation::acquire(ImplReference* ref)
{
   // set reference and increase count
   mReference = ref;
   if(mReference != NULL)
   {
      mReference->count++;
   }
}

void Operation::release()
{
   // decrement reference count
   if(mReference != NULL && --mReference->count == 0)
   {
      // delete implementation and reference
      delete mReference->impl;
      delete mReference;
      mReference = NULL;
   }
}

Operation& Operation::operator=(const Operation& rhs)
{
   if(this != &rhs)
   {
      // release old reference and acquire new one
      release();
      acquire(rhs.mReference);
   }
   
   return *this;
}

bool Operation::operator==(const Operation& rhs)
{
   bool rval = false;
   
   if(this == &rhs)
   {
      rval = true;
   }
   else if(this->mReference == rhs.mReference)
   {
      rval = true;
   }
   
   return rval;
}

bool Operation::operator!=(const Operation& rhs)
{
   return !(*this == rhs);
}

OperationImpl& Operation::operator*()
{
   return *mReference->impl;
}

OperationImpl* Operation::operator->()
{
   return mReference->impl;
}
