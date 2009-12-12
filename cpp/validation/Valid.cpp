/*
 * Copyright (c) 2008-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/validation/Valid.h"

using namespace monarch::rt;
using namespace monarch::validation;

Valid::Valid()
{
}

Valid::~Valid()
{
}

bool Valid::isValid(
   DynamicObject& obj,
   ValidatorContext* context)
{
   return true;
}
