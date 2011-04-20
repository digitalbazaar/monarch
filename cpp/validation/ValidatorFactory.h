/*
 * Copyright (c) 2011 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_validation_ValidatorFactory_H
#define monarch_validation_ValidatorFactory_H

#include "monarch/validation/Validator.h"

#include "monarch/io/InputStream.h"
#include "monarch/rt/SharedLock.h"
#include "monarch/util/StringTools.h"

namespace monarch
{
namespace validation
{

/**
 * A ValidatorFactory is used to create custom Validators. Custom Validators
 * can be defined by writing JSON files and loading them with a factory. Once a
 * custom Validator's definition has been loaded by a factory, that factory can
 * then instantiate that Validator by its type.
 *
 * @author Dave Longley
 */
class ValidatorFactory
{
public:
   /**
    * A create Validator function.
    */
   typedef ValidatorRef (*CreateValidatorFunc)(
      ValidatorFactory*, monarch::rt::DynamicObject&);

   /**
    * Data for validating a definition and creating a Validator.
    */
   struct ValidatorDef
   {
      ValidatorRef validator;
      CreateValidatorFunc function;
   };

   /**
    * A map of built-in validator type to validator definition.
    */
   typedef std::map<const char*, ValidatorDef, monarch::util::StringComparator>
      ValidatorDefMap;

   /**
    * A map of custom validator type to validator.
    */
   typedef std::map<const char*, ValidatorRef, monarch::util::StringComparator>
      ValidatorMap;

   /**
    * A lock for sychronizing definition loading.
    */
   monarch::rt::SharedLock* mLoadLock;

protected:
   /**
    * The Validator definition map.
    */
   ValidatorDefMap mValidatorDefs;

   /**
    * The defined validators.
    */
   ValidatorMap mValidators;

public:
   /**
    * Creates a new ValidatorFactory.
    *
    * @param sync true if this ValidatorFactory needs thread-synchronization
    *           support, false if not.
    */
   ValidatorFactory(bool sync = false);

   /**
    * Destructs this ValidatorFactory.
    */
   virtual ~ValidatorFactory();

   /**
    * Loads JSON-formatted Validator definition file(s). If the given path
    * contains a directory, an attempt will be made to load all files in the
    * directory. If the given path contains a file, only that file will be
    * loaded.
    *
    * @param path the path(s) to definition file(s).
    *
    * @return true on success, false on failure with exception set.
    */
   virtual bool loadValidatorDefinitions(const char* path);

   /**
    * Loads JSON-formatted Validator definition from an input stream.
    *
    * @param is the input stream to read the definition from.
    *
    * @return true on success, false on failure with exception set.
    */
   virtual bool loadValidatorDefinition(monarch::io::InputStream* is);

   /**
    * Loads JSON-formatted Validator definition from a DynamicObject.
    *
    * @param def the DynamicObject with the definition.
    *
    * @return true on success, false on failure with exception set.
    */
   virtual bool loadValidatorDefinition(monarch::rt::DynamicObject& def);

   /**
    * Creates a new Validator of the given type. The type must be a previously
    * previously defined custom Validator type.
    *
    * @param type the type of Validator to create.
    *
    * @return the Validator if successful, NULL if not with exception set.
    */
   virtual ValidatorRef createValidator(const char* type);

   /**
    * Called internally to creates a Validator from a Validator definition.
    * This method should only be called from a CreateValidatorFunc. To load
    * a Validator definition from a DynamicObject, call loadValidatorDefinition
    * with a DynamicObject definition.
    *
    * @param def the definition to use.
    *
    * @return the created Validator.
    */
   virtual ValidatorRef createValidator(monarch::rt::DynamicObject& def);

protected:
   /**
    * Gets a definition validator.
    *
    * @param type the type of the definition validator.
    *
    * @return the definition validator, NULL if not found.
    */
   virtual ValidatorRef getDefValidator(const char* type);

   /**
    * Defines Validators for all of the given definitions.
    *
    * @param defs a map of the definitions.
    *
    * @return true on success, false on failure with exception set.
    */
   virtual bool defineValidators(monarch::rt::DynamicObject& defs);

   /**
    * Recursively validates the given definition and gathers its dependencies
    *
    * @param def the definition to validate.
    * @param deps the dependencies for the definition.
    *
    * @return true on success, false on failure with exception set.
    */
   virtual bool recursiveValidate(
      monarch::rt::DynamicObject& def, monarch::rt::DynamicObject& deps);
};

// type definition for a reference counted ValidatorFactory
typedef monarch::rt::Collectable<ValidatorFactory> ValidatorFactoryRef;

} // end namespace validation
} // end namespace monarch
#endif
