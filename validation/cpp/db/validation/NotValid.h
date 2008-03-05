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
