/*
 * Copyright (c) 2008-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_validation_Regex_H
#define monarch_validation_Regex_H

#include "monarch/validation/Validator.h"

namespace monarch
{
namespace validation
{

/**
 * Validates the target object with a regular expression.  The target must be a
 * string and an automatic Type(String) validator is applied to test for this
 * pre-condition.
 *
 * @author David I. Lehn
 */
class Regex : public Validator
{
protected:
   /* regex string */
   char* mRegex;

   /* automatic string validator */
   Validator* mStringValidator;

public:
   /**
    * Creates a new validator.  If regex is NULL will match against the pattern
    * "^$".
    *
    * @param regex the regular expression to use
    * @param errorMessage custom error message
    */
   Regex(const char* regex, const char* errorMessage = NULL);

   /**
    * Destructs this validator.
    */
   virtual ~Regex();

   /**
    * Checks if an object is valid.  Object must be a string.
    *
    * @param obj the object to validate.
    * @param context context to use during validation.
    *
    * @return true if obj is valid, false and exception set otherwise.
    */
   virtual bool isValid(
      monarch::rt::DynamicObject& obj,
      ValidatorContext* context);
   using Validator::isValid;
};

} // end namespace validation
} // end namespace monarch
#endif
