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
 * Validates the type of an object.
 *
 * d => 0
 *
 *   Type ti(Int32);
 *   ti.isValid(d) => true
 *
 *   Type ts(String);
 *   ts.isValid(d) => false
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
    * Checks if an object is valid.
    * 
    * @param obj the object to validate.
    * @param context context to use during validation.
    * 
    * @return true if obj is valid, false and exception set otherwise.
    */
   virtual bool isValid(
      db::rt::DynamicObject& obj,
      ValidatorContext* context);
   using Validator::isValid;
};

} // end namespace validation
} // end namespace db
#endif
