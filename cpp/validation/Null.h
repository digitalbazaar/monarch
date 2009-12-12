/*
 * Copyright (c) 2008-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef db_validation_Null_H
#define db_validation_Null_H

#include "monarch/validation/Validator.h"

namespace db
{
namespace validation
{

/**
 * Validates the target object as being null.
 *
 * @author Dave Longley
 */
class Null : public Validator
{
public:
   /**
    * Creates a new validator.
    *
    * @param errorMessage custom error message.
    */
   Null(const char* errorMessage = NULL);

   /**
    * Destructs this validator.
    */
   virtual ~Null();

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
} // end namespace db
#endif
