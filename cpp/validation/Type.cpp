/*
 * Copyright (c) 2008-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/validation/Type.h"

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
      const char* strType =
         obj.isNull() ?
            "null" :
            DynamicObject::descriptionForType(obj->getType());

      DynamicObject detail = context->addError("db.validation.TypeError", &obj);
      detail["validator"] = "db.validator.Type";
      // FIXME: localize -- lehn
      // FIXME: really? do we need to mention this, because we'd have to
      //        do this for every string in the system.. -- manu
      detail["message"] = mErrorMessage ? mErrorMessage :
         "The given object type is different from the " \
         "required object type.";
      detail["givenType"] = strType;
      detail["requiredType"] = DynamicObject::descriptionForType(mType);
   }
   else
   {
      context->addSuccess();
   }

   return rval;
}
