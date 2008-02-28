/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/validation/Valid.h"

using namespace db::rt;
using namespace db::validation;

Valid::Valid()
{
}

Valid::~Valid()
{
}

bool Valid::isValid(
   DynamicObject& obj,
   DynamicObject* state,
   std::vector<const char*>* path)
{
   return true;
}
