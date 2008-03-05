/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_validation_All_H
#define db_validation_All_H

#include "db/validation/ValidatorList.h"

namespace db
{
namespace validation
{

/**
 * Validates a list of Validators such that all sub-validators must be valid.
 * Validation checking short circuits if a sub-validator returns false.
 * 
 * @author David I. Lehn
 */
class All : public ValidatorList
{
public:
   /**
    * Creates a new validator.
    */
   All(Validator* validator, ...);
   
   /**
    * Destructs this validator.
    */
   virtual ~All();

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
   using Validator::isValid;
};

} // end namespace validation
} // end namespace db
#endif
