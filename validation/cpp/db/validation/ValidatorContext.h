/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_validation_ValidatorContext_H
#define db_validation_ValidatorContext_H

#include "db/rt/DynamicObject.h"
#include <string>
#include <vector>
#include <cstdarg>

namespace db
{
namespace validation
{

/**
 * Context to be used during validation.  This object is used to keep track
 * of the "path" while Validators traverse a DynamicObject.  It can also
 * store arbitrary state as needed for validation.
 * 
 * @author David I. Lehn
 */
class ValidatorContext
{
protected:
   /* Arbitrary user state.  Lazily created as needed. */
   db::rt::DynamicObject* mState;
   
   /* Path used during object traversal. */
   std::vector<const char*>* mPath;
   
   /* Flag to stop setting of exceptoins.  Useful when only concerned with
    * success of failure of validators rather than details.
    */
   bool mSetExceptions;
    
public:
   /**
    * Creates a new ValidatorContext.
    */
   ValidatorContext(db::rt::DynamicObject* state = NULL);
   
   /**
    * Destructs this ValidatorContext.
    */
   virtual ~ValidatorContext();
   
   /**
    * Get arbitrary user state for this context.  If the context was not
    * initialized with state then a new state object is created and freed upon
    * context termination.
    * 
    * @return state object for this context.
    */
   virtual db::rt::DynamicObject& getState();

   /**
    * Control if exceptions are set with addError().
    * 
    * @param set true to set exceptions with addError, false to make addError
    *            a no-op.
    *
    * @return previous exception setting value.
    */
   virtual bool setExceptions(bool set);

   /**
    * Push a new element onto the current path.
    * 
    * @param path new element to push onto the path.  path must stay valid
    *             until poped with popPath() or the context is finished.
    */
   virtual void pushPath(const char* path);
   
   /**
    * Pop last element off the current path.
    */
   virtual void popPath();
   
   /**
    * Get the depth of the path created from pushPath() and popPath().
    * 
    * @return current path depth.
    */
   virtual unsigned int getDepth();
   
   /**
    * Get the current path as a string or "" if no path.  The return value is
    * created by concatinating all elements added with pushPath().
    * 
    * @return string current path as a string.  "" if no path.
    */
   virtual std::string getPath();
   
   /**
    * Creates an Error exception if needed and add a basic error report with
    * the given "type" field and a "message" field with "Invalid value!".  Use
    * the returned object to change the message or add additional details as
    * needed.
    * 
    * setException() can be used to stop this method from setting exceptions.
    * 
    * @param type type of this error in "db.validation.MaxError" format.
    * 
    * @return DynamicObject to fill with details.
    */
   virtual db::rt::DynamicObject addError(const char* type);
};

} // end namespace validation
} // end namespace db
#endif
