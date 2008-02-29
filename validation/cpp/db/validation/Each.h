/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_validation_Each_H
#define db_validation_Each_H

#include "db/validation/Validator.h"

namespace db
{
namespace validation
{

/**
 * Validates each member of an array or value of a map with a single
 * sub-validator.  All members must validate for this validator to return
 * true.  All members will be checked.
 * 
 * @author David I. Lehn
 */
class Each : public Validator
{
protected:
   /* Validator to use on each member */
   Validator* mValidator;

   /**
    * Validates an array.
    * 
    * @param obj the object to validate.
    * @param state arbitrary state for validators to use during validation.
    * @param path the validation path used to get to this validator.
    * 
    * @return true if obj is valid, false and exception set otherwise.
    */
   bool isArrayValid(
      db::rt::DynamicObject& obj,
      db::rt::DynamicObject* state,
      std::vector<const char*>* path);

   /**
    * Validates a map.
    * 
    * @param obj the object to validate.
    * @param state arbitrary state for validators to use during validation.
    * @param path the validation path used to get to this validator.
    * 
    * @return true if obj is valid, false and exception set otherwise.
    */
   bool isMapValid(
      db::rt::DynamicObject& obj,
      db::rt::DynamicObject* state,
      std::vector<const char*>* path);

public:
   /**
    * Creates a new validator.
    */
   Each(Validator* validator);
   
   /**
    * Destructs this validator.
    */
   virtual ~Each();
   
   /**
    * Checks if an object is valid.
    * 
    * @param obj the object to validate.  Must be an Array or Map.
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
