/*
 * Copyright (c) 2008-2009 Digital Bazaar, Inc. All rights reserved.
 */
#define __STDC_LIMIT_MACROS
#define __STDC_CONSTANT_MACROS

#include "monarch/validation/Int.h"

#include "monarch/rt/DynamicObjectIterator.h"

#include <inttypes.h>

using namespace std;
using namespace monarch::rt;
using namespace monarch::validation;

Int::Int(int64_t min, int64_t max, const char* errorMessage) :
   Validator(errorMessage)
{
   bool minneg = (min < 0);
   uint64_t absmin = (minneg ? -min : min);
   bool maxneg = (max < 0);
   uint64_t absmax = (maxneg ? -max : max);
   setMinMax(absmin, minneg, absmax, maxneg);
}

Int::Int(uint64_t min, bool minNegative, uint64_t max, bool maxNegative,
   const char* errorMessage)
{
   setMinMax(min, minNegative, max, maxNegative);
}

Int::Int(IntegerType type, const char* errorMessage) :
   Validator(errorMessage)
{
   switch(type)
   {
      case Negative:
         setMinMax(UINT64_MAX, true, 1, true);
         break;
      case NonPositive:
         setMinMax(UINT64_MAX, true, 0, false);
         break;
      case Zero:
         setMinMax(0, false, 0, false);
         break;
      case NonNegative:
         setMinMax(0, false, UINT64_MAX, false);
         break;
      case Positive:
         setMinMax(1, false, UINT64_MAX, false);
         break;
   }
}

Int::Int(DynamicObjectType type, const char* errorMessage) :
   Validator(errorMessage)
{
   switch(type)
   {
      case Int32:
         setMinMax(UINT64_C(0x80000000), true, INT32_MAX, false);
         break;
      case UInt32:
         setMinMax(0, false, UINT32_MAX, false);
         break;
      case Int64:
         setMinMax(UINT64_C(0x8000000000000000), true, INT64_MAX, false);
         break;
      case UInt64:
         setMinMax(0, false, UINT64_MAX, false);
         break;
      default:
         // FIXME: better way to handle type error?
         setMinMax(UINT64_MAX, true, UINT64_MAX, false);
         break;
   }
}

Int::Int(const char* errorMessage) :
   Validator(errorMessage)
{
   setMinMax(UINT64_MAX, true, UINT64_MAX, false);
}

Int::~Int()
{
}

void Int::setMinMax(
   uint64_t min, bool minNegative, uint64_t max, bool maxNegative)
{
   mMin = min;
   mMinNegative = minNegative;
   mMax = max;
   mMaxNegative = maxNegative;
}

bool Int::isValid(
   DynamicObject& obj,
   ValidatorContext* context)
{
   bool rval;
   DynamicObjectType objType;

   rval = !obj.isNull();

   if(rval)
   {
      objType = obj->getType();
      if(objType == String)
      {
         objType = DynamicObject::determineType(obj->getString());
      }

      // type check
      rval =
         objType == Int32 ||
         objType == UInt32 ||
         objType == Int64 ||
         objType == UInt64;
   }

   if(!rval)
   {
      DynamicObject detail =
         context->addError("monarch.validation.ValueError", &obj);
      detail["validator"] = "monarch.validator.Int";
      detail["message"] =
         mErrorMessage ? mErrorMessage :
            "The given value type is required to be an integer.";
   }

   // absolute value of dyno value
   uint64_t val = 0;
   // flag if val is negative
   bool valneg = false;

   // get value for min/max check
   if(rval)
   {
      // get value and sign
      switch(objType)
      {
         case Int32:
         case Int64:
         {
            int64_t raw = obj->getInt64();
            valneg = (raw < 0);
            val = (uint64_t)(valneg ? -raw : raw);
            break;
         }
         case UInt32:
         case UInt64:
         {
            valneg = false;
            val = obj->getUInt64();
            break;
         }
         default:
            // never get here
            break;
      }
   }

   // min check
   if(rval)
   {
      if(mMinNegative != valneg)
      {
         // signs are different
         // val meets the minimum unless val is negative
         rval = !valneg;
      }
      else
      {
         // signs are the same
         // val meets the minimum if:
         // 1. val is positive and larger than mMin
         // 2. val is negative and smaller than mMin
         rval = (!valneg ? val >= mMin : val <= mMin);
      }

      // set exception on failure
      if(!rval)
      {
         DynamicObject detail =
            context->addError("monarch.validation.ValueError", &obj);
         detail["validator"] = "monarch.validator.Int";
         detail["message"] = mErrorMessage ? mErrorMessage :
            "The given integer value is less than the required minimum "
            "integer value.";
         detail["expectedMin"] = mMin;
      }
   }

   // max check
   if(rval)
   {
      if(mMaxNegative != valneg)
      {
         // signs are different
         // val meets the maximum unless val is positive
         rval = valneg;
      }
      else
      {
         // signs are the same
         // val meets the maximum if:
         // 1. val is positive and smaller than mMax
         // 2. val is negative and larger than mMax
         rval = (valneg ? val >= mMax : val <= mMax);
      }

      // set exception on failure
      if(!rval)
      {
         DynamicObject detail =
            context->addError("monarch.validation.ValueError", &obj);
         detail["validator"] = "monarch.validator.Int";
         detail["message"] = mErrorMessage ? mErrorMessage :
            "The given integer value is greater than the allowable maximum "
            "integer value.";
         detail["expectedMax"] = mMax;
      }
   }

   if(rval)
   {
      context->addSuccess();
   }

   return rval;
}
