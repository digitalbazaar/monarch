/*
 * Copyright (c) 2012 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_validation_Contains_h
#define monarch_validation_Contains_h

#include "monarch/validation/Validator.h"

namespace monarch
{
namespace validation
{

/**
 * Validates if either a target object equals a validation object or if the
 * target is an array or map and contains the validation object.
 *
 * @author David I. Lehn
 */
class Contains : public Validator
{
protected:
   /* Object with valid object */
   monarch::rt::DynamicObject mObject;

public:
   /**
    * Creates a new validator.
    *
    * @param errorMessage custom error message
    */
   Contains(
      monarch::rt::DynamicObject& object, const char* errorMessage = NULL);

   /**
    * Destructs this validator.
    */
   virtual ~Contains();

   /**
    * Checks if an object is valid.
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
