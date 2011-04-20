/*
 * Copyright (c) 2008-2011 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_validation_Array_H
#define monarch_validation_Array_H

#include "monarch/validation/Validator.h"
#include <vector>
#include <utility>

namespace monarch
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
   struct Entry
   {
      Validator* validator;
      ValidatorRef reference;
   };
   typedef std::vector<std::pair<int,Entry> > ValidatorPairs;
   ValidatorPairs mValidators;

public:
   /**
    * Creates a new validator.
    */
   Array();

   /**
    * Creates a new validator with a -1 index terminated index:validator list.
    * Use this constructor if the order of the validators matters.
    *
    * @param index the index for the first validator to check.
    * @param validator the first validator to check.
    * @param ... more index:validator pairs.
    */
   Array(int index, Validator* validator, ...);

   /**
    * Creates a new validator with a NULL terminated validator list. Use
    * this constructor if order of the validators does not matter.
    *
    * @param validator the first validator to check.
    * @param ... more validators.
    */
   Array(Validator* validator, ...);

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
      monarch::rt::DynamicObject& obj,
      ValidatorContext* context);
   using Validator::isValid;

   /**
    * Returns the number of validators in the array.
    *
    * @return the length of the validator.
    */
   virtual size_t length();

   /**
    * Adds an index:validator pair.
    *
    * @param index an array index, -1 to check any index.
    * @param validator a Validator.
    */
   virtual void addValidator(int index, Validator* validator);

   /**
    * Adds an index:validator pair.
    *
    * @param index an array index, -1 to check any index.
    * @param validator a Validator.
    */
   virtual void addValidatorRef(int index, ValidatorRef validator);

   /**
    * Adds index:validator pairs.
    *
    * @param index an array index.
    * @param validator the first validator to check.
    * @param ap a vararg list.
    */
   virtual void addValidators(int index, Validator* validator, va_list ap);

   /**
    * Adds a -1 terminated list of index:validator pairs.
    *
    * @param index an array index.
    * @param validator the first validator to check.
    * @param ... more index:validator pairs.
    */
   virtual void addValidators(int index, Validator* validator, ...);

   /**
    * Adds -1:validator pairs.
    *
    * @param validator the first validator.
    * @param ap a vararg list.
    */
   virtual void addValidators(Validator* validator, va_list ap);

   /**
    * Adds a NULL terminated list of -1:validator pairs.
    *
    * @param validator the first validator.
    * @param ... more -1:validator pairs.
    */
   virtual void addValidators(Validator* validator, ...);
};

} // end namespace validation
} // end namespace monarch
#endif
