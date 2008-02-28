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
 * Validate all members of a map.
 * NOTE: Checking for extra members not yet implemented.
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
   
   /**
    * Add a key:validator pair.
    * 
    * @param key a map key.
    * @param validator a Validator.
    */
   virtual void addValidator(const char* key, Validator* validator);
   
   /**
    * Add key:validator pairs.
    * 
    * @param key a map key.
    * @param ap a vararg list.
    */
   virtual void addValidators(const char* key, va_list ap);
   
   /**
    * Add a NULL terminated list of key:validator pairs.
    * 
    * @param key a map key.
    * @param ... more key:validator pairs.
    */
   virtual void addValidators(const char* key, ...);
};

} // end namespace validation
} // end namespace db
#endif
