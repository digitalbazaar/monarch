/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/validation/Min.h"

using namespace db::rt;
using namespace db::validation;

Min::Min(int size) :
   mSize(size)
{
}

Min::~Min()
{
}

bool Min::isValid(
   DynamicObject& obj,
   DynamicObject* state,
   std::vector<const char*>* path)
{
   bool rval = obj->length() >= mSize;
   
   if(!rval)
   {
      DynamicObject detail = addError(path, "db.validation.MinError");
      detail["expectedMin"] = mSize;
   }
   
   return rval;
}
