/*
 * Copyright (c) 2008-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_validation_Max_H
#define monarch_validation_Max_H

#include "monarch/validation/Validator.h"

namespace monarch
{
namespace validation
{

/**
 * Validates the maximum size of a string, array, or map.
 *
 * NOTE: For numeric types this checks against the storage size.
 *
 * @author David I. Lehn
 */
class Max : public Validator
{
protected:
   /* Size limit */
   int mSize;

public:
   /**
    * Creates a new validator.
    *
    * @param errorMessage custom error message
    */
   Max(int size, const char* errorMessage = NULL);

   /**
    * Destructs this validator.
    */
   virtual ~Max();

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
