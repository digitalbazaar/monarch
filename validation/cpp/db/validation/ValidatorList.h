/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_validation_ValidatorList_H
#define db_validation_ValidatorList_H

#include "db/validation/Validator.h"
#include <vector>

namespace db
{
namespace validation
{

/**
 * Base class for Validators that use a list of Validators.
 */
class ValidatorList : public Validator
{
protected:
   std::vector<Validator*> mValidators;

public:
   /**
    * Creates a new ValidatorList.
    */
   ValidatorList();
   
   /**
    * Creates a new ValidatorList with a NULL terminated list of Validators.
    * 
    * @param validator a Validator
    * @param ... more Validators.
    */
   ValidatorList(Validator* validator, ...);
   
   /**
    * Destructs this ValidatorList.
    */
   virtual ~ValidatorList();
   
   /**
    * Add a validator to the list.
    * 
    * @param validator a Validator.
    */
   virtual void addValidator(Validator* validator);
   
   /**
    * Add validators to the list.
    * 
    * @param validator a Validator.
    * @param ap a vararg list.
    */
   virtual void addValidators(Validator* validator, va_list ap);
   
   /**
    * Add a NULL terminated list of validators to the list.
    * 
    * @param validator a Validator.
    * @param ... more Validators.
    */
   virtual void addValidators(Validator* validator, ...);
};

} // end namespace validation
} // end namespace db
#endif
