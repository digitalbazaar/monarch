/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_validation_Map_H
#define db_validation_Map_H

#include "db/validation/Validator.h"
#include <vector>
#include <utility>

namespace db
{
namespace validation
{

/**
 * Validates specific members of a DynamicObject Map.
 *
 * For many values set up the validator with varargs:
 *
 *   Map m(
 *      "username", new Type(String),
 *      "password", new Type(String),
 *      NULL);
 *   m.isValid(dyno);
 *
 * NOTE: Checking for arbitrary extra members not yet implemented.  However,
 * you can set a validator such as: "new Optional(new NotValid())" in order
 * to fail on specific members that are not allowed.
 * 
 * @author David I. Lehn
 */
class Map : public Validator
{
protected:
   std::vector<std::pair<const char*,Validator*> > mValidators;

public:
   /**
    * Creates a new validator.
    */
   Map();
   
   /**
    * Creates a new validator with a NULL key terminated key:validator list.
    */
   Map(const char* key, ...);
   
   /**
    * Destructs this validator.
    */
   virtual ~Map();
   
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
   
   /**
    * Adds a key:validator pair.
    * 
    * @param key a map key.
    * @param validator a Validator.
    */
   virtual void addValidator(const char* key, Validator* validator);
   
   /**
    * Adds key:validator pairs.
    * 
    * @param key a map key.
    * @param ap a vararg list.
    */
   virtual void addValidators(const char* key, va_list ap);
   
   /**
    * Adds a NULL terminated list of key:validator pairs.
    * 
    * @param key a map key.
    * @param ... more key:validator pairs.
    */
   virtual void addValidators(const char* key, ...);
};

} // end namespace validation
} // end namespace db
#endif
