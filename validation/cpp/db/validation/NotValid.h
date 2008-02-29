/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_validation_NotValid_H
#define db_validation_NotValid_H

#include "db/validation/Validator.h"

namespace db
{
namespace validation
{

/**
 * A validator that always fails.
 * 
 * @author David I. Lehn
 */
class NotValid : public Validator
{
public:
   /**
    * Creates a new validator.
    */
   NotValid();
   
   /**
    * Destructs this validator.
    */
   virtual ~NotValid();
   
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
