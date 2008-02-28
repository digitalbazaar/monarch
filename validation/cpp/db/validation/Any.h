/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_validation_Any_H
#define db_validation_Any_H

#include "db/validation/ValidatorList.h"

namespace db
{
namespace validation
{
   
/**
 * Validate a list of Validators such that at least one sub-validators must be
 * valid.  Validation checking short circuits when an invalid validator found.
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
    * Validate an object.
    * 
    * @param obj the object to validate.
    * @param state arbitrary state for validators to use during validation.
    * @param path the validation path used to get to this validator.
    * 
    * @return true if obj is valid, false and exception set otherwise.
    */
   virtual bool isValid(
      db::rt::DynamicObject& obj,
      db::rt::DynamicObject* state = NULL,
      std::vector<const char*>* path = NULL);
};

} // end namespace validation
} // end namespace db
#endif
