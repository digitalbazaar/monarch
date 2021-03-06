/*
 * Copyright (c) 2008-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_validation_Any_H
#define monarch_validation_Any_H

#include "monarch/validation/ValidatorList.h"

namespace monarch
{
namespace validation
{

/**
 * Validates a list of Validators such that at least one sub-validators must be
 * valid.  Validation checking short circuits if a sub-validator returns true.
 *
 * @author David I. Lehn
 */
class Any : public ValidatorList
{
public:
   /**
    * Creates a new validator.
    */
   Any(Validator* validator, ...);

   /**
    * Destructs this validator.
    */
   virtual ~Any();

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
