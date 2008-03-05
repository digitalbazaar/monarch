/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_validation_Min_H
#define db_validation_Min_H

#include "db/validation/Validator.h"

namespace db
{
namespace validation
{

/**
 * Validates the minimum size of a string, array, or map.
 * 
 * NOTE: For numeric types this checks against the storage size.
 *
 * @author David I. Lehn
 */
class Min : public Validator
{
protected:
   /* Size limit */
   int mSize;
   
public:
   /**
    * Creates a new validator.
    */
   Min(int size);
   
   /**
    * Destructs this validator.
    */
   virtual ~Min();
   
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
