/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/validation/Type.h"

using namespace db::rt;
using namespace db::validation;

Type::Type(db::rt::DynamicObjectType type, const char* errorMessage) :
   Validator(errorMessage),
   mType(type)
{
}

Type::~Type()
{
}

bool Type::isValid(DynamicObject& obj, ValidatorContext* context)
{
   bool rval = (!obj.isNull() && obj->getType() == mType);
   
   if(!rval)
   {
      const char* strType = DynamicObject::descriptionForType(obj->getType());
      int length = 40 + strlen(strType);
      char temp[length];
      snprintf(temp, length, "Invalid type, received '%s'", strType);

      DynamicObject detail = context->addError("db.validation.TypeError");
      // FIXME: localize
      detail["message"] = mErrorMessage ? mErrorMessage : temp;
      detail["expectedType"] = DynamicObject::descriptionForType(mType);
   }
   return rval;
}
