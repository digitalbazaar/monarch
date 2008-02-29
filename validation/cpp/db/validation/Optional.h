/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_validation_Optional_H
#define db_validation_Optional_H

#include "db/validation/Validator.h"

namespace db
{
namespace validation
{

/**
 * A container that can be used for optional Map validation.  A Map's key
 * validator can be wrapped in an Optional validator if it does not need to be
 * present for successful validation.  If it is present then the sub-validator
 * is checked.
 * 
 * @author David I. Lehn
 */
class Optional : Validator
{
protected:
   Validator* mValidator;

public:
   /**
    * Creates a new validator.
    */
   Optional(Validator* validator);
   
   /**
    * Destructs this validator.
    */
   virtual ~Optional();
   
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

   /**
    * If an object is missing is this Check if this validator is optional.
    * 
    * @param state arbitrary state for validators to use during validation.
    * 
    * @return true if optional, false if mandatory.
    */
   virtual bool isOptional(db::rt::DynamicObject* state = NULL);
};

} // end namespace validation
} // end namespace db
#endif
