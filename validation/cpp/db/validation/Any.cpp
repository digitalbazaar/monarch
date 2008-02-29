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
   db::rt::DynamicObject* state,
   std::vector<const char*>* path)
{
   bool rval = false;
   
   std::vector<Validator*>::iterator i;
   for(i = mValidators.begin();
      !rval && i != mValidators.end();
      i++)
   {
      rval |= (*i)->isValid(obj, state, path);
   }
   
   return rval;
}
