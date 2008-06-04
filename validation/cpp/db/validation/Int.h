/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_validation_Int_H
#define db_validation_Int_H

#include "db/validation/Validator.h"

namespace db
{
namespace validation
{

/**
 * Validates an object is an integer.  Optionally can specify if the value is
 * signed or minimum and maximum values.
 * 
 * @author David I. Lehn
 */
class Int : public Validator
{
protected:
   /* minimum */
   int64_t mMin;
   
   /* maximum */
   uint64_t mMax;
   
public:
   /**
    * Creates a new validator with specified minimum and maximum.
    * 
    * @param min minimum integer value
    * @param max maximum integer value
    * @param errorMessage custom error message
    */
   Int(int64_t min, uint64_t max, const char* errorMessage = NULL);
   
   /**
    * Creates a new validator with specifed signedness.
    * 
    * @param isSigned true for signed integers, false for unsigned
    * @param errorMessage custom error message
    */
   Int(bool isSigned, const char* errorMessage = NULL);
   
   /**
    * Creates a new validator.
    * 
    * @param errorMessage custom error message
    */
   Int(const char* errorMessage = NULL);
   
   /**
    * Destructs this validator.
    */
   virtual ~Int();
   
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
