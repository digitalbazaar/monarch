/*
 * Copyright (c) 2008-2009 Digital Bazaar, Inc. All rights reserved.
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
 * Validates specific members of a DynamicObject Array.
 *
 * For many values set up the validator with varargs:
 *
 *   Array a(
 *      0, new Type(Int32),
 *      1, new Type(Int32),
 *      -1);
 *   a.isValid(dyno);
 *
 * NOTE: Checking for arbitrary extra members not yet implemented.  However,
 * you can set a validator such as: "new Optional(new NotValid())" in order
 * to fail on specific members that are not allowed.
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
    * Adds an index:validator pair.
    *
    * @param index an array index.
    * @param validator a Validator.
    */
   virtual void addValidator(int index, Validator* validator);

   /**
    * Adds index:validator pairs.
    *
    * @param index an array index.
    * @param ap a vararg list.
    */
   virtual void addValidators(int index, va_list ap);

   /**
    * Adds a -1 terminated list of index:validator pairs.
    *
    * @param index an array index.
    * @param ... more key:validator pairs.
    */
   virtual void addValidators(int index, ...);
};

} // end namespace validation
} // end namespace db
#endif
