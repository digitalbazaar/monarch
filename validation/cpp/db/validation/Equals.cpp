/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/validation/Equals.h"

using namespace db::rt;
using namespace db::validation;

Equals::Equals(db::rt::DynamicObject& object) :
   mObject(object)
{
}

Equals::~Equals()
{
}

bool Equals::isValid(
   db::rt::DynamicObject& obj,
   db::rt::DynamicObject* state,
   std::vector<const char*>* path)
{
   bool rval = obj == mObject;
   if(!rval)
   {
      addError(path, "db.validation.EqualityFailure");
      // FIXME: add expected value to error detail?
   }
   return rval;
}
