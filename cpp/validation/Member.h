/*
 * Copyright (c) 2010 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_validation_Member_H
#define monarch_validation_Member_H

#include "monarch/validation/Validator.h"

namespace monarch
{
namespace validation
{

/**
 * Validates that a specific member of a DynamicObject Map exists, otherwise
 * sets a customizable error. Member validators should be specified at the
 * end of a list of validators for a particular map so that they will override
 * the default error messages if a field is not found.
 *
 * Usage:
 *
 *   All v(
 *      new Map(
 *         "mycheckbox", new Equals("yes"),
 *         NULL),
 *      new Member("mycheckbox", "You didn't check 'mycheckbox'"),
 *      NULL);
 *   v.isValid(dyno);
 *
 * @author Dave Longley
 */
class Member : public Validator
{
protected:
   /**
    * The name of the member to check for.
    */
   char* mKey;

public:
   /**
    * Creates a new validator.
    *
    * @param key the map key to look for.
    * @param errorMessage a custom error message.
    */
   Member(const char* key, const char* errorMessage = NULL);

   /**
    * Destructs this validator.
    */
   virtual ~Member();

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
