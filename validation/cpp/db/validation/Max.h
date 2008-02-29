/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_validation_Max_H
#define db_validation_Max_H

#include "db/validation/Validator.h"

namespace db
{
namespace validation
{

/**
 * Validates the maximum size of a string, array, or map.
 *
 * NOTE: For numeric types this checks against the storage size.
 * 
 * @author David I. Lehn
 */
class Max : public Validator
{
protected:
   /* Size limit */
   int mSize;
   
public:
   /**
    * Creates a new validator.
    */
   Max(int size);
   
   /**
    * Destructs this validator.
    */
   virtual ~Max();
   
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
