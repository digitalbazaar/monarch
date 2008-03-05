/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/validation/Compare.h"

using namespace db::rt;
using namespace db::validation;

Compare::Compare(const char* key0, const char* key1) :
   mKey0(key0),
   mKey1(key1)
{
}

Compare::~Compare()
{
}

bool Compare::isValid(
   db::rt::DynamicObject& obj,
   ValidatorContext* context)
{
   bool rval = obj[mKey0] == obj[mKey1];

   if(!rval)
   {
      if(context->getDepth() != 0)
      {
         context->pushPath(".");
      }
      
      context->pushPath(mKey1);
      DynamicObject detail = context->addError("db.validation.CompareFailure");
      detail["expectedValue"] = obj[mKey0];
      context->popPath();
      
      if(context->getDepth() == 1)
      {
         context->popPath();
      }
   }
   return rval;
}
