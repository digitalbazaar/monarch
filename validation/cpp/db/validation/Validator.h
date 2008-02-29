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
    * Checks if an object is valid.
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
    * Returns if this validator is optional or mandatory.  This method will
    * be called from the Map validator if a key is missing from a target object
    * during a validation check.  This allows for specialized validators to
    * be skipped.  The default implementation assumes validators are mandatory
    * and returns false.  The Optional validator returns true and can be used
    * as a wrapper around other validators.
    *
    * See the Optional class documentation for an example.
    * 
    * @param state arbitrary state for validators to use during validation.
    * 
    * @return true if optional, false if mandatory.
    */
   virtual bool isOptional(db::rt::DynamicObject* state = NULL);
   
   /**
    * Makes a string path out of a vector of path components.
    * 
    * @param path vector of strings to concatenate.
    * 
    * @return string representing the path.
    */
   static std::string stringFromPath(std::vector<const char*>* path);
   
   /**
    * Creates an Error exception if needed and add a basic error report with
    * the given "type" field and a "message" field with "Invalid value!".  Use
    * the returned object to change the message or add additional details as
    * needed.
    * 
    * @param path vector of strings to concatinate.
    * @param type type of this error in "db.validation.MaxError" format.
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
