/*
 * Copyright (c) 2008-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "db/validation/CompareText.h"

#include "db/util/StringTools.h"

using namespace db::rt;
using namespace db::util;
using namespace db::validation;

CompareText::CompareText(const char* text, const char* errorMessage) :
   Validator(errorMessage),
   mText(text)
{
   // replace all CRLF with LF
   StringTools::replaceAll(mText, "\r\n", "\n");
   StringTools::replaceAll(mText, "\r", "\n");
}

CompareText::~CompareText()
{
}

bool CompareText::isValid(DynamicObject& obj, ValidatorContext* context)
{
   bool rval = !obj.isNull();

   if(rval)
   {
      // replace all CRLF with LF
      std::string compare = obj->getString();
      StringTools::replaceAll(compare, "\r\n", "\n");
      StringTools::replaceAll(compare, "\r", "\n");

      rval = (strcmp(compare.c_str(), mText.c_str()) == 0);
      if(!rval)
      {
         DynamicObject detail = context->addError(
            "db.validation.CompareTextFailure", &obj);
         detail["validator"] = "db.validator.CompareText";
         detail["message"] = (mErrorMessage != NULL ?
            mErrorMessage :
            "The two text strings that were given are required "
            "to be equivalent, but they are different.");
         detail["expectedValue"] = mText.c_str();
      }
   }

   if(rval)
   {
      context->addSuccess();
   }

   return rval;
}
