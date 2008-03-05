/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/validation/Any.h"

using namespace db::rt;
using namespace db::validation;

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
   db::rt::DynamicObject& obj,
   ValidatorContext* context)
{
   bool rval = false;
   
   // store previous value
   bool setExceptions = context->setExceptions(false);
   std::vector<Validator*>::iterator i;
   for(i = mValidators.begin();
      !rval && i != mValidators.end();
      i++)
   {
      rval = rval || (*i)->isValid(obj, context);
   }
   // restore
   context->setExceptions(setExceptions);
   
   // set exception for Any 
   if(!rval)
   {
      // FIXME: Use the confusing "AnyError" name instead?
      // or set a more appropriate detail message? 
      context->addError("db.validation.ValueError");
   }
   
   return rval;
}
