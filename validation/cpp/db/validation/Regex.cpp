/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/util/regex/Pattern.h"
#include "db/validation/Regex.h"
#include "db/validation/Type.h"

#include <cstdlib>

using namespace db::rt;
using namespace db::util::regex;
using namespace db::validation;

Regex::Regex(const char* regex, const char* errorMessage) :
   Validator(errorMessage)
{
   mRegex = regex ? strdup(regex) : strdup("^$");
   mStringValidator = new Type(String);
}

Regex::~Regex()
{
   if(mRegex)
   {
      free(mRegex);
   }
   delete mStringValidator;
}

bool Regex::isValid(
   db::rt::DynamicObject& obj,
   ValidatorContext* context)
{
   bool rval = mStringValidator->isValid(obj, context);

   if(rval)
   {
      // FIXME compile the regex
      rval = Pattern::match(mRegex, obj->getString());
      if(!rval)
      {
         DynamicObject detail =
            context->addError("db.validation.ValueError", &obj);
         detail["validator"] = "db.validator.Regex";
         if(mErrorMessage)
         {
            detail["message"] = mErrorMessage;
         }
      }
   }

   return rval;
}
