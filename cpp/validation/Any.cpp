/*
 * Copyright (c) 2008-2011 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/validation/Any.h"

#include "monarch/rt/Exception.h"

using namespace monarch::rt;
using namespace monarch::validation;

Any::Any(Validator* validator, ...)
{
   va_list ap;

   va_start(ap, validator);
   addValidators(validator, ap);
   va_end(ap);
}

Any::~Any()
{
}

bool Any::isValid(
   monarch::rt::DynamicObject& obj,
   ValidatorContext* context)
{
   bool rval = false;

   // store previous set exceptions value
   bool setExceptions = context->setExceptions(false);

   // validate each "any" validator until one is valid, for each
   // invalid result, store the context results if they match or
   // exceed the most number of successful validations
   uint32_t maxSuccesses = 0;
   DynamicObject errors;
   errors->setType(Array);
   for(std::vector<Validator*>::iterator i = mValidators.begin();
       !rval && i != mValidators.end(); ++i)
   {
      rval = (*i)->isValid(obj, context);
      if(!rval)
      {
         // get validation results
         DynamicObject r = context->getResults();
         uint32_t successes = r["successes"]->getUInt32();
         if(successes > maxSuccesses)
         {
            // new max successes
            maxSuccesses = successes;
            errors->clear();
            errors->append(r["errors"]);
         }
         else if(successes == maxSuccesses)
         {
            errors->append(r["errors"]);
         }

         // clear validation results
         context->clearResults();
      }
   }

   // restore set exceptions value
   context->setExceptions(setExceptions);

   // set exception for Any, return array of errors
   if(!rval)
   {
      DynamicObject detail =
         context->addError("monarch.validation.ValueError", &obj);
      detail["validator"] = "monarch.validator.Any";
      detail["possibleErrors"] = errors;

      // add first set of specific errors to exception as most likely
      // error case out of possible errors
      if(setExceptions)
      {
         ExceptionRef e = Exception::get();
         e->getDetails()["errors"].merge(errors[0], false);
      }
   }

   return rval;
}
