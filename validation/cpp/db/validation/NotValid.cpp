/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/validation/NotValid.h"

using namespace db::rt;
using namespace db::validation;

NotValid::NotValid()
{
}

NotValid::~NotValid()
{
}

bool NotValid::isValid(
   DynamicObject& obj,
   DynamicObject* state,
   std::vector<const char*>* path)
{
   addError(path, "db.validation.NotValid");
   return false;
}
