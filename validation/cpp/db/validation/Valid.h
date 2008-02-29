/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_validation_Valid_H
#define db_validation_Valid_H

#include "db/validation/Validator.h"

namespace db
{
namespace validation
{

/**
 * A validator which always succeeds.
 * 
 * @author David I. Lehn
 */
class Valid : public Validator
{
public:
   /**
    * Creates a new validator.
    */
   Valid();
   
   /**
    * Destructs this validator.
    */
   virtual ~Valid();
   
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
