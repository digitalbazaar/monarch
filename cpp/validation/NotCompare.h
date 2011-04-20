/*
 * Copyright (c) 2008-2011 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_validation_NotCompare_H
#define monarch_validation_NotCompare_H

#include "monarch/validation/Validator.h"
#
namespace monarch
{
namespace validation
{

/**
 * Validates equality of the values for two keys of a DynamicObject Map.
 *
 * d => { "aKey": aValue, "bKey": bValue }
 *
 * Validate aValue and bValue are the same (via ==) with:
 *
 *   NotCompare c("aKey", "bKey");
 *   c.isValid(d)
 *
 * @author David I. Lehn
 */
class NotCompare : public Validator
{
protected:
   /* First key to use */
   char* mKey0;

   /* Second key to use */
   char* mKey1;

public:
   /**
    * Creates a new validator.
    *
    * @param key0 first key to use
    * @param key1 second key to use
    * @param errorMessage custom error message
    */
   NotCompare(
      const char* key0, const char* key2, const char* errorMessage = NULL);

   /**
    * Destructs this validator.
    */
   virtual ~NotCompare();

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
