/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/validation/Validator.h"

using namespace db::rt;
using namespace db::validation;

Validator::Validator()
{
}

Validator::~Validator()
{
}

bool Validator::isValid(
      db::rt::DynamicObject& obj,
      ValidatorContext* context)
{
   return true;
}

bool Validator::isValid(db::rt::DynamicObject& obj)
{
   bool rval;
   ValidatorContext* ctx = new ValidatorContext();
   rval = isValid(obj, ctx);
   delete ctx;
   return rval; 
}

bool Validator::isOptional(ValidatorContext* context)
{
   return false;
}
