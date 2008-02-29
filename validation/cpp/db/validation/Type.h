/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_validation_Type_H
#define db_validation_Type_H

#include "db/validation/Validator.h"

namespace db
{
namespace validation
{

/**
 * Validate the type of an object.
 * 
 * @author David I. Lehn
 */
class Type : public Validator
{
protected:
   db::rt::DynamicObjectType mType;

public:
   /**
    * Creates a new validator.
    */
   Type(db::rt::DynamicObjectType type);
   
   /**
    * Destructs this validator.
    */
   virtual ~Type();
   
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
