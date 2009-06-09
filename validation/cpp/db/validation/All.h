/*
 * Copyright (c) 2008-2009 Digital Bazaar, Inc. All rights reserved.
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
 * This validator can also be used to mask invalid data in its sub-validators,
 * which is particularly important when validating secure data fields like
 * passwords.
 * 
 * @author David I. Lehn
 */
class All : public ValidatorList
{
protected:
   /**
    * The mask type for this validator.
    */
   ValidatorContext::MaskType mMaskType;
   
public:
   /**
    * Creates a new validator.
    */
   All(Validator* validator, ...);
   
   /**
    * Creates a new validator.
    * 
    * @param mt the type of error data to mask.
    */
   All(ValidatorContext::MaskType mt, Validator* validator, ...);
   
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
