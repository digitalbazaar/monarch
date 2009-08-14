/*
 * Copyright (c) 2008-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "db/validation/Array.h"

using namespace db::rt;
using namespace db::validation;

Array::Array()
{
}

Array::Array(int index, ...)
{
   va_list ap;

   va_start(ap, index);
   addValidators(index, ap);
   va_end(ap);
}

Array::~Array()
{
   std::vector<std::pair<int,Validator*> >::iterator i;

   for(i = mValidators.begin();
      i != mValidators.end();
      i++)
   {
      delete i->second;
   }
}

bool Array::isValid(
   DynamicObject& obj,
   ValidatorContext* context)
{
   bool rval = true;

   if(!obj.isNull() && obj->getType() == db::rt::Array)
   {
      std::vector<std::pair<int,Validator*> >::iterator i;
      for(i = mValidators.begin(); i != mValidators.end(); i++)
      {
         if(obj->length() >= i->first)
         {
            // add [#] indexing to path even if at root
            char idx[23];
            snprintf(idx, 23, "[%d]", i->first);
            context->pushPath(idx);

            // do not short-circuit
            if(!i->second->isValid(obj[i->first], context))
            {
               rval = false;
            }
            context->popPath();
         }
         else
         {
            rval = false;
            DynamicObject detail =
               context->addError("db.validation.MissingIndex", &obj);
            detail["validator"] = "db.validator.Array";
            detail["index"] = i->first;
         }
      }
   }
   else
   {
      rval = false;
      DynamicObject detail =
         context->addError("db.validation.TypeError", &obj);
      detail["validator"] = "db.validator.Array";
      detail["message"] = "The given object type must be an Array.";
   }

   if(rval)
   {
      context->addSuccess();
   }

   return rval;
}

void Array::addValidator(int index, Validator* validator)
{
   mValidators.push_back(std::make_pair(index, validator));
}

void Array::addValidators(int index, va_list ap)
{
   while(index != -1)
   {
      Validator* v = va_arg(ap, Validator*);
      addValidator(index, v);
      index = va_arg(ap, int);
   }
}

void Array::addValidators(int index, ...)
{
   va_list ap;

   va_start(ap, index);
   addValidators(index, ap);
   va_end(ap);
}
