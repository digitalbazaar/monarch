/*
 * Copyright (c) 2008-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_validation_ValidatorContext_H
#define monarch_validation_ValidatorContext_H

#include "monarch/rt/DynamicObject.h"
#include <string>
#include <vector>
#include <cstdarg>

namespace monarch
{
namespace validation
{

/**
 * Context to be used during validation. This object is used to keep track
 * of the "path" while Validators traverse a DynamicObject. It can also
 * store arbitrary state as needed for validation.
 *
 * @author David I. Lehn
 */
class ValidatorContext
{
public:
   // types of error data that can be masked
   enum MaskType
   {
      MaskNone          = 0,
      MaskInvalidValues = 1 << 0
   };

protected:
   /**
    * Path used during object traversal.
    */
   std::vector<char*>* mPath;

   /**
    * Flag to stop setting of exceptions. Useful when only concerned with
    * success or failure of validators rather than details.
    */
   bool mSetExceptions;

   /**
    * The current mask type for hiding error data.
    */
   MaskType mMaskType;

   /**
    * Stores the results of the validation process. This includes the
    * number of successful validations and any errors that occurred
    * during the validation process, regardless of whether or not
    * exceptions are being set. This is particularly useful for
    * producing helpful error messages for special validators like Any.
    */
   monarch::rt::DynamicObject mResults;

public:
   /**
    * Creates a new ValidatorContext.
    */
   ValidatorContext();

   /**
    * Destructs this ValidatorContext.
    */
   virtual ~ValidatorContext();

   /**
    * Sets the mask type for error data. This will control what data will
    * appear in errors and what will not. This is particular useful for
    * hidding invalid values for data fields like passwords.
    *
    * @param mt the new mask type.
    */
   virtual void setMaskType(MaskType mt);

   /**
    * Gets the current mask type for error data. This controls what data
    * will appear in errors and what will not. This is particular useful for
    * hidding invalid values for data fields like passwords.
    *
    * @return the current mask type.
    */
   virtual MaskType getMaskType();

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
    * Records a successful validation.
    */
   virtual void addSuccess();

   /**
    * Creates an Error exception if needed and add a basic error report with
    * the given "type" field and a "message" field with "Invalid value!".  If
    * the object parameter is given it will be assigned to the "invalidValue"
    * field.  Use the returned object to change the message or add additional
    * details as needed.
    *
    * setException() can be used to stop this method from setting exceptions.
    *
    * @param type type of this error in "monarch.validation.MaxError" format.
    * @param object object that caused this error.
    *
    * @return a DynamicObject to fill with details.
    */
   virtual monarch::rt::DynamicObject addError(
      const char* type, monarch::rt::DynamicObject* object = NULL);

   /**
    * Gets the current validation results.
    */
   virtual monarch::rt::DynamicObject getResults();

   /**
    * Clears the current validation results from this context.
    */
   virtual void clearResults();
};

} // end namespace validation
} // end namespace monarch
#endif
