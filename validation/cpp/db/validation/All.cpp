/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/validation/All.h"

using namespace db::rt;
using namespace db::validation;

All::All(Validator* validator, ...)
{
   va_list ap;
   
   va_start(ap, validator);
   addValidators(validator, ap);
   va_end(ap);
}
   
All::~All()
{
}

bool All::isValid(
   db::rt::DynamicObject& obj,
   ValidatorContext* context)
{
   bool rval = true;
   
   std::vector<Validator*>::iterator i;
   for(i = mValidators.begin();
      rval && i != mValidators.end();
      i++)
   {
      rval = rval && (*i)->isValid(obj, context);
   }
   
   return rval;
}
