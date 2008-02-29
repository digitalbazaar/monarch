/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_validation_Validator_H
#define db_validation_Validator_H

#include "db/rt/DynamicObject.h"
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
public:
   /**
    * Creates a new Validator.
    */
   Validator();
   
   /**
    * Destructs this Validator.
    */
   virtual ~Validator();
   
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
      std::vector<const char*>* path = NULL) = 0;

   /**
    * If an object is missing is this Check if this validator is optional.
    * 
    * @param state arbitrary state for validators to use during validation.
    * 
    * @return true if optional, false if mandatory.
    */
   virtual bool isOptional(db::rt::DynamicObject* state = NULL)
   {
      return false;
   }
   
   /**
    * Make a string path out of a vector of path components.
    * 
    * @param path vector of strings to concatinate.
    * 
    * @return string representing the path.
    */
   static std::string stringFromPath(std::vector<const char*>* path);
   
   /**
    * Create an Error exception if needed and add a basic error report with the
    * given "type" field and a "message" field with "Invalid value.".  Use
    * the returned object to add a more detailed message or aditional details
    * as needed.
    * 
    * @param path vector of strings to concatinate.
    * @param type type of this error in format like "db.validation.MaxError"
    * 
    * @return DynamicObject to fill with details.
    */
   static db::rt::DynamicObject addError(
      std::vector<const char*>* path,
      const char* type);
};

} // end namespace validation
} // end namespace db
#endif
