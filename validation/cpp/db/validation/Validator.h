/*
 * Copyright (c) 2008-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef db_validation_Validator_H
#define db_validation_Validator_H

#include "db/rt/DynamicObject.h"
#include "db/validation/ValidatorContext.h"
#include <string>
#include <vector>
#include <cstdarg>

namespace db
{
namespace validation
{

/**
 * Base functionality for Validators.
 *
 * @author David I. Lehn
 */
class Validator
{
protected:
   /* Custom error message */
   char* mErrorMessage;

public:
   /**
    * Creates a new Validator with a specified error message or NULL to use
    * a default message for this validator.
    *
    * @param errorMessage a custom error message used in the error detail
    *        "message" field.  NULL to use default.  string is copied.
    */
   Validator(const char* errorMessage = NULL);

   /**
    * Destructs this Validator.
    */
   virtual ~Validator();

   /**
    * Checks if an object is valid.
    *
    * @param obj the object to validate.
    * @param context context to use during validation.
    *
    * @return true if obj is valid, false and exception set otherwise.
    */
   virtual bool isValid(
      db::rt::DynamicObject& obj,
      ValidatorContext* context);

   /**
    * Checks if an object is valid using a new context.
    *
    * @param obj the object to validate.
    *
    * @return true if obj is valid, false and exception set otherwise.
    */
   virtual bool isValid(db::rt::DynamicObject& obj);

   /**
    * Returns if this validator is optional or mandatory.  This method will
    * be called from the Map validator if a key is missing from a target object
    * during a validation check.  This allows for specialized validators to
    * be skipped.  The default implementation assumes validators are mandatory
    * and returns false.  The Optional validator returns true and can be used
    * as a wrapper around other validators.
    *
    * See the Optional class documentation for an example.
    *
    * @param state arbitrary state for validators to use during validation.
    *
    * @return true if optional, false if mandatory.
    */
   virtual bool isOptional(ValidatorContext* context);
};

// type definition for a reference counted Validator
typedef db::rt::Collectable<Validator> ValidatorRef;

} // end namespace validation
} // end namespace db
#endif
