/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_validation_Compare_H
#define db_validation_Compare_H

#include "db/validation/Validator.h"
#
namespace db
{
namespace validation
{

/**
 * Validate equality of an object.
 * 
 * @author David I. Lehn
 */
class Compare : public Validator
{
protected:
   /* First key to use */
   const char* mKey0;
   
   /* Second key to use */
   const char* mKey1;

public:
   /**
    * Creates a new validator.
    * 
    * @param key0 first key to use
    * @param key1 second key to use
    */
   Compare(const char* key0, const char* key2);
   
   /**
    * Destructs this validator.
    */
   virtual ~Compare();
   
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
