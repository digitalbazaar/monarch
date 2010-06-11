/*
 * Copyright (c) 2008-2010 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/validation/All.h"

using namespace monarch::rt;
using namespace monarch::validation;

All::All(Validator* validator, ...) :
   mMaskType(ValidatorContext::MaskNone)
{
   va_list ap;

   va_start(ap, validator);
   addValidators(validator, ap);
   va_end(ap);
}

All::All(ValidatorContext::MaskType mt, Validator* validator, ...) :
   mMaskType(mt)
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
   monarch::rt::DynamicObject& obj,
   ValidatorContext* context)
{
   bool rval = true;

   // save mask type for context
   ValidatorContext::MaskType mt = context->getMaskType();

   // set mask type for context
   context->setMaskType(mMaskType);

   // make sure all validators are tested, do not short-circuit
   std::vector<Validator*>::iterator i;
   for(i = mValidators.begin(); i != mValidators.end(); ++i)
   {
      if(!(*i)->isValid(obj, context))
      {
         rval = false;
      }
   }

   if(rval)
   {
      context->addSuccess();
   }

   // restore mask type
   context->setMaskType(mt);

   return rval;
}
