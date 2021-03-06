/*
 * Copyright (c) 2008-2011 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/validation/Array.h"

#include "monarch/rt/DynamicObjectIterator.h"
#include <cstdio>

using namespace monarch::rt;
using namespace monarch::validation;

Array::Array()
{
}

Array::Array(Validator* validator, ...)
{
   va_list ap;

   va_start(ap, validator);
   addValidators(validator, ap);
   va_end(ap);
}

Array::Array(int index, Validator* validator, ...)
{
   va_list ap;

   va_start(ap, validator);
   addValidators(index, validator, ap);
   va_end(ap);
}

Array::~Array()
{
   for(ValidatorPairs::iterator i = mValidators.begin();
       i != mValidators.end(); ++i)
   {
      // only clean up validator if it has no reference
      if(i->second.reference.isNull())
      {
         delete i->second.validator;
      }
   }
}

bool Array::isValid(
   DynamicObject& obj,
   ValidatorContext* context)
{
   bool rval = true;

   if(!obj.isNull() && obj->getType() == monarch::rt::Array)
   {
      for(ValidatorPairs::iterator i = mValidators.begin();
          i != mValidators.end(); ++i)
      {
         // index does not matter
         if(i->first == -1)
         {
            /* Note: More than one array element validators might pass on the
             * same element in the current implementation, which might be
             * unexpected behavior. If it is, then some additional state should
             * be created to store all of the elements that passed a particular
             * validators. Then that state should be reduced until each
             * validator has one element that matches. This is the success
             * case. This might make the error case more difficult -- see
             * the Any validator for the complexities there. Keep in mind that
             * this validator is different from the combination of an Each
             * and an Any (which would validate each element in the array),
             * this validator instead just makes sure that certain elements
             * exist in a particular array.
             */

            // store previous set exceptions value
            bool setExceptions = context->setExceptions(false);

            // look for a valid element in the array
            bool found = false;
            DynamicObjectIterator ii = obj.getIterator();
            while(!found && ii->hasNext())
            {
               DynamicObject& next = ii->next();

               // add [#] indexing to path
               char idx[23];
               snprintf(idx, 23, "[%d]", ii->getIndex());
               context->pushPath(idx);

               // short-circuit on pass
               found = i->second.validator->isValid(next, context);

               // only set exception if this is the last element
               if(!found && !ii->hasNext())
               {
                  rval = false;
                  context->setExceptions(setExceptions);
                  i->second.validator->isValid(next, context);
                  context->setExceptions(false);
               }
               context->popPath();
            }

            // restore set exceptions value
            context->setExceptions(setExceptions);
         }
         // index matters
         else if(obj->length() >= i->first)
         {
            // add [#] indexing to path even if at root
            char idx[23];
            snprintf(idx, 23, "[%d]", i->first);
            context->pushPath(idx);

            // do not short-circuit
            if(!i->second.validator->isValid(obj[i->first], context))
            {
               rval = false;
            }
            context->popPath();
         }
         else
         {
            rval = false;
            DynamicObject detail =
               context->addError("monarch.validation.MissingIndex", &obj);
            detail["validator"] = "monarch.validator.Array";
            detail["index"] = i->first;
         }
      }
   }
   else
   {
      rval = false;
      DynamicObject detail =
         context->addError("monarch.validation.TypeError", &obj);
      detail["validator"] = "monarch.validator.Array";
      detail["message"] = "The given object type must be an Array.";
   }

   if(rval)
   {
      context->addSuccess();
   }

   return rval;
}

size_t Array::length()
{
   return mValidators.size();
}

void Array::addValidator(int index, Validator* validator)
{
   Entry e = {&(*validator), NULL};
   mValidators.push_back(std::make_pair(index, e));
}

void Array::addValidatorRef(int index, ValidatorRef validator)
{
   Entry e = {&(*validator), validator};
   mValidators.push_back(std::make_pair(index, e));
}

void Array::addValidators(int index, Validator* validator, va_list ap)
{
   while(index != -1)
   {
      if(validator == NULL)
      {
         validator = va_arg(ap, Validator*);
      }
      addValidator(index, validator);
      index = va_arg(ap, int);
      validator = NULL;
   }
}

void Array::addValidators(int index, Validator* validator, ...)
{
   va_list ap;

   va_start(ap, validator);
   addValidators(index, validator, ap);
   va_end(ap);
}

void Array::addValidators(Validator* validator, va_list ap)
{
   while(validator != NULL)
   {
      addValidator(-1, validator);
      validator = va_arg(ap, Validator*);
   }
}

void Array::addValidators(Validator* validator, ...)
{
   va_list ap;

   va_start(ap, validator);
   addValidators(validator, ap);
   va_end(ap);
}
