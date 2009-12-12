/*
 * Copyright (c) 2008-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/util/regex/Pattern.h"
#include "monarch/validation/Regex.h"
#include "monarch/validation/Type.h"

#include <cstdlib>

using namespace monarch::rt;
using namespace monarch::util::regex;
using namespace monarch::validation;

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
   monarch::rt::DynamicObject& obj,
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
      else
      {
         context->addSuccess();
      }
   }

   return rval;
}
