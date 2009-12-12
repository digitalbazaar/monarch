/*
 * Copyright (c) 2008-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/validation/ValidatorList.h"

using namespace monarch::rt;
using namespace monarch::validation;

ValidatorList::ValidatorList()
{
}

ValidatorList::ValidatorList(Validator* validator, ...)
{
   va_list ap;

   va_start(ap, validator);
   addValidators(validator, ap);
   va_end(ap);
}

ValidatorList::~ValidatorList()
{
   std::vector<Validator*>::iterator i;
   for(i = mValidators.begin();
      i != mValidators.end();
      i++)
   {
      delete (*i);
   }
}

void ValidatorList::addValidator(Validator* validator)
{
   mValidators.push_back(validator);
}

void ValidatorList::addValidators(Validator* validator, va_list ap)
{
   while(validator != NULL)
   {
      addValidator(validator);
      validator = va_arg(ap, Validator*);
   }
}

void ValidatorList::addValidators(Validator* validator, ...)
{
   va_list ap;

   va_start(ap, validator);
   addValidators(validator, ap);
   va_end(ap);
}
