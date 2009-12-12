/*
 * Copyright (c) 2008-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_validation_Type_H
#define monarch_validation_Type_H

#include "monarch/validation/Validator.h"

namespace monarch
{
namespace validation
{

/**
 * Validates the type of an object.
 *
 * d => 0
 *
 *   Type ti(Int32);
 *   ti.isValid(d) => true
 *
 *   Type ts(String);
 *   ts.isValid(d) => false
 *
 * @author David I. Lehn
 */
class Type : public Validator
{
protected:
   monarch::rt::DynamicObjectType mType;

public:
   /**
    * Creates a new validator.
    *
    * @param errorMessage custom error message
    */
   Type(monarch::rt::DynamicObjectType type, const char* errorMessage = NULL);

   /**
    * Destructs this validator.
    */
   virtual ~Type();

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
