/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */
#define __STDC_LIMIT_MACROS

#include "db/validation/Int.h"
#include "db/rt/DynamicObjectIterator.h"

#include <inttypes.h>

using namespace std;
using namespace db::rt;
using namespace db::validation;

Int::Int(int64_t min, uint64_t max, const char* errorMessage) :
   Validator(errorMessage),
   mMin(min),
   mMax(max)
{
}

Int::Int(bool isSigned, const char* errorMessage) :
   Validator(errorMessage),
   mMin(INT64_MIN),
   mMax(UINT64_MAX)
{
   if(!isSigned)
   {
      mMin = 0;
   }
}

Int::Int(const char* errorMessage) :
   Validator(errorMessage),
   mMin(INT64_MIN),
   mMax(UINT64_MAX)
{
}

Int::~Int()
{
}

bool Int::isValid(
   DynamicObject& obj,
   ValidatorContext* context)
{
   bool rval;
   DynamicObjectType objType;

   objType = obj->getType();
   if(objType == String)
   {
      objType = obj.determineType(obj->getString());
   }

   // type check
   rval =
      objType == UInt64 ||
      objType == Int64 ||
      objType == UInt32 ||
      objType == Int32;
   
   if(!rval)
   {
      DynamicObject detail = context->addError("db.validation.ValueError");
      detail["message"] = mErrorMessage ? mErrorMessage : "Value not an integer!";
   }
   
   // min check
   if(rval)
   {
      int64_t val = obj->getInt64();
      rval = val >= mMin;
      if(!rval)
      {
         DynamicObject detail = context->addError("db.validation.ValueError");
         detail["message"] = mErrorMessage ? mErrorMessage : "Value too small!";
      }
   }
   
   // max check
   if(rval)
   {
      uint64_t val = obj->getUInt64();
      rval = val <= mMax;
      if(!rval)
      {
         DynamicObject detail = context->addError("db.validation.ValueError");
         detail["message"] = mErrorMessage ? mErrorMessage : "Value too large!";
      }
   }
   
   return rval;
}
