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
 * Validate each member of an array or value of a map.
 * 
 * @author David I. Lehn
 */
class Each : public Validator
{
protected:
   /* Validator to use on each member */
   Validator* mValidator;

   /**
    * Validate an array.
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
    * Validate a map.
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
