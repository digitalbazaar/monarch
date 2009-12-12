/*
 * Copyright (c) 2008-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_validation_Valid_H
#define monarch_validation_Valid_H

#include "monarch/validation/Validator.h"

namespace monarch
{
namespace validation
{

/**
 * A validator which always succeeds.
 *
 * @author David I. Lehn
 */
class Valid : public Validator
{
public:
   /**
    * Creates a new validator.
    */
   Valid();

   /**
    * Destructs this validator.
    */
   virtual ~Valid();

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
