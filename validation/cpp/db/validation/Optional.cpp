/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/validation/Optional.h"

using namespace db::rt;
using namespace db::validation;

Optional::Optional(Validator* validator) :
   mValidator(validator)
{
}

Optional::~Optional()
{
   delete mValidator;
}

bool Optional::isValid(
   DynamicObject& obj,
   DynamicObject* state,
   std::vector<const char*>* path)
{
   return mValidator->isValid(obj, state, path);
}

bool Optional::isOptional(db::rt::DynamicObject* state)
{
   return true;
}
