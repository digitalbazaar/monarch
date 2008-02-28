/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_validation_Array_H
#define db_validation_Array_H

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
class Array : public Validator
{
protected:
   std::vector<std::pair<int,Validator*> > mValidators;

public:
   /**
    * Creates a new validator.
    */
   Array();
   
   /**
    * Creates a new validator with a -1 index terminated index:validator list.
    */
   Array(int index, ...);
   
   /**
    * Destructs this validator.
    */
   virtual ~Array();
   
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
    * Add an index:validator pair.
    * 
    * @param index an array index.
    * @param validator a Validator.
    */
   virtual void addValidator(int index, Validator* validator);
   
   /**
    * Add index:validator pairs.
    * 
    * @param index an array index.
    * @param ap a vararg list.
    */
   virtual void addValidators(int index, va_list ap);
   
   /**
    * Add a -1 terminated list of index:validator pairs.
    * 
    * @param index an array index.
    * @param ... more key:validator pairs.
    */
   virtual void addValidators(int index, ...);
};

} // end namespace validation
} // end namespace db
#endif
