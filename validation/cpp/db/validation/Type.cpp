/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/validation/Type.h"

using namespace db::rt;
using namespace db::validation;

Type::Type(db::rt::DynamicObjectType type) :
   mType(type)
{
}

Type::~Type()
{
}

bool Type::isValid(
   DynamicObject& obj,
   DynamicObject* state,
   std::vector<const char*>* path)
{
   bool rval = obj->getType() == mType;
   if(!rval)
   {
      DynamicObject detail = addError(path, "db.validation.TypeError");
      // FIXME: localize
      detail["message"] = "Invalid type!";
      detail["expectedType"] = DynamicObject::descriptionForType(mType);
      
      ExceptionRef e = new Exception();
   }
   return rval;
}
