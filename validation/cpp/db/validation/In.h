/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_validation_In_H
#define db_validation_In_H

#include "db/validation/Validator.h"

namespace db
{
namespace validation
{

/**
 * Validates an object is in an array or is a key in a map.
 * 
 * @author David I. Lehn
 */
class In : public Validator
{
protected:
   /* Object with valid contents */
   db::rt::DynamicObject mContents;

public:
   /**
    * Creates a new validator.
    */
   In(db::rt::DynamicObject& contents);
   
   /**
    * Destructs this validator.
    */
   virtual ~In();
   
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
      std::vector<const char*>* path = NULL);
};

} // end namespace validation
} // end namespace db
#endif
