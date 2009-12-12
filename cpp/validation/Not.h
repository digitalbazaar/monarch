/*
 * Copyright (c) 2008-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef db_validation_Not_H
#define db_validation_Not_H

#include "monarch/validation/Validator.h"

namespace db
{
namespace validation
{

/**
 * Negates the result of a sub-validator.  Exceptions from the sub-validator
 * are supressed.
 *
 * @author David I. Lehn
 */
class Not : public Validator
{
protected:
   /* sub-validator to use  */
   Validator* mValidator;

public:
   /**
    * Creates a new validator.
    */
   Not(Validator* validator, const char* errorMessage = NULL);

   /**
    * Destructs this validator.
    */
   virtual ~Not();

   /**
    * Checks if an object is invalid.
    *
    * @param obj the object to validate.  Must be an Array or Map.
    * @param context context to use during validation.
    *
    * @return true if obj is valid, false and exception set otherwise.
    */
   virtual bool isValid(
      db::rt::DynamicObject& obj,
      ValidatorContext* context);
   using Validator::isValid;
};

} // end namespace validation
} // end namespace db
#endif
