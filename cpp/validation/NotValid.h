/*
 * Copyright (c) 2008-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_validation_NotValid_H
#define monarch_validation_NotValid_H

#include "monarch/validation/Validator.h"

namespace monarch
{
namespace validation
{

/**
 * A validator that always fails.
 *
 * @author David I. Lehn
 */
class NotValid : public Validator
{
public:
   /**
    * Creates a new validator.
    *
    * @param errorMessage custom error message
    */
   NotValid(const char* errorMessage = NULL);

   /**
    * Destructs this validator.
    */
   virtual ~NotValid();

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
