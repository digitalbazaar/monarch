/*
 * Copyright (c) 2011 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/validation/ValidatorFactory.h"

#include "monarch/data/json/JsonReader.h"
#include "monarch/io/FileInputStream.h"
#include "monarch/io/FileList.h"
#include "monarch/rt/DynamicObjectIterator.h"
#include "monarch/validation/Validation.h"

using namespace monarch::data::json;
using namespace monarch::io;
using namespace monarch::rt;
using namespace monarch::util;
namespace v = monarch::validation;

#define VF_EXCEPTION "monarch.validation.ValidatorFactory"

#define MO_VALIDATOR_ALL         "All"
#define MO_VALIDATOR_ANY         "Any"
#define MO_VALIDATOR_ARRAY       "Array"
#define MO_VALIDATOR_COMPARE     "Compare"
#define MO_VALIDATOR_EACH        "Each"
#define MO_VALIDATOR_EQUALS      "Equals"
#define MO_VALIDATOR_IN          "In"
#define MO_VALIDATOR_INT         "Int"
#define MO_VALIDATOR_MAP         "Map"
#define MO_VALIDATOR_MAX         "Max"
#define MO_VALIDATOR_MEMBER      "Member"
#define MO_VALIDATOR_MIN         "Min"
#define MO_VALIDATOR_NOT         "Not"
#define MO_VALIDATOR_NOT_COMPARE "NotCompare"
#define MO_VALIDATOR_NOT_VALID   "NotValid"
#define MO_VALIDATOR_REGEX       "Regex"
#define MO_VALIDATOR_TYPE        "Type"
#define MO_VALIDATOR_VALID       "Valid"
#define MO_VALIDATOR_NULL        "Null"

/* Define validators for validating definition files */

static v::Validator* _baseValidator()
{
   return new v::Map(
      "type", new v::Type(String),
      "def", new v::Optional(new v::Valid()),
      "optional", new v::Optional(new v::Type(Boolean)),
      "error", new v::Optional(new v::Type(String)),
      NULL);
}

static v::Validator* _customValidator(bool topLevel)
{
   return topLevel ?
      static_cast<v::Validator*>(new v::Map(
         "type", new v::Type(String),
         "extends", new v::Any(
            new v::Equals(MO_VALIDATOR_ALL),
            new v::Equals(MO_VALIDATOR_ANY),
            new v::Equals(MO_VALIDATOR_ARRAY),
            new v::Equals(MO_VALIDATOR_COMPARE),
            new v::Equals(MO_VALIDATOR_EACH),
            new v::Equals(MO_VALIDATOR_EQUALS),
            new v::Equals(MO_VALIDATOR_IN),
            new v::Equals(MO_VALIDATOR_INT),
            new v::Equals(MO_VALIDATOR_MAP),
            new v::Equals(MO_VALIDATOR_MAX),
            new v::Equals(MO_VALIDATOR_MEMBER),
            new v::Equals(MO_VALIDATOR_MIN),
            new v::Equals(MO_VALIDATOR_NOT),
            new v::Equals(MO_VALIDATOR_NOT_COMPARE),
            new v::Equals(MO_VALIDATOR_NOT_VALID),
            new v::Equals(MO_VALIDATOR_REGEX),
            new v::Equals(MO_VALIDATOR_TYPE),
            new v::Equals(MO_VALIDATOR_VALID),
            new v::Equals(MO_VALIDATOR_NULL),
            NULL),
         "def", new v::Optional(new v::Valid()),
         "optional", new v::Optional(new v::Type(Boolean)),
         "error", new v::Optional(new v::Type(String)),
         NULL)) :
      // cannot define custom validators that aren't top-level
      static_cast<v::Validator*>(new v::All(
         new v::Map(
            "optional", new v::Optional(new v::Type(Boolean)),
            NULL),
         new v::Not(new v::Member(
            "def",
            "A custom Validator must be defined before it is nested.")),
         new v::Not(new v::Member(
            "extends",
            "A custom Validator must be defined before it is nested.")),
         new v::Not(new v::Member(
            "error",
            "A custom Validator must be defined before it is nested.")),
         NULL));
}

static v::Validator* _allValidator()
{
   return new v::Map(
      "def", new v::All(
         new v::Type(Array),
         new v::Each(_baseValidator()),
         NULL),
      NULL);
}

static v::Validator* _anyValidator()
{
   return new v::Map(
      "def", new v::All(
         new v::Type(Array),
         new v::Each(_baseValidator()),
         NULL),
      NULL);
}

static v::Validator* _arrayValidator()
{
   return new v::Map(
      "def", new v::Optional(new v::All(
         new v::Type(Array),
         new v::Each(new v::All(
            _baseValidator(),
            new v::Map(
               "index", new v::Optional(new v::Int(v::Int::NonNegative)),
               NULL),
            NULL)),
         NULL)),
      NULL);
}

static v::Validator* _compareValidator()
{
   return new v::Map(
      "def", new v::Map(
         "key1", new v::Type(String),
         "key2", new v::Type(String),
         NULL),
      NULL);
}

static v::Validator* _eachValidator()
{
   return new v::Map(
      "def", _baseValidator());
}

static v::Validator* _equalsValidator()
{
   return new v::Map(
      "def", new v::Valid(),
      NULL);
}

static v::Validator* _inValidator()
{
   return new v::Map(
      "def", new v::Any(
         new v::Type(Map),
         new v::Type(Array),
         NULL),
      NULL);
}

static v::Validator* _intValidator()
{
   return new v::Map(
      "def", new v::Optional(new v::Any(
         new v::Map(
            "min", new v::Int(),
            "max", new v::Int(),
            NULL),
         new v::Map(
            "type", new v::Any(
               new v::Equals("Zero"),
               new v::Equals("Positive"),
               new v::Equals("Negative"),
               new v::Equals("NonPositive"),
               new v::Equals("NonNegative"),
               NULL),
            NULL),
        NULL)),
      NULL);
}

static v::Validator* _mapValidator()
{
   return new v::Map(
      "def", new v::Optional(new v::All(
         new v::Type(Map),
         new v::Each(_baseValidator()),
         NULL)),
      NULL);
}

static v::Validator* _maxValidator()
{
   return new v::Map(
      "def", new v::Int(v::Int::NonNegative),
      NULL);
}

static v::Validator* _memberValidator()
{
   return new v::Map(
      "def", new v::Type(String),
      NULL);
}

static v::Validator* _minValidator()
{
   return new v::Map(
      "def", new v::Int(v::Int::NonNegative),
      NULL);
}

static v::Validator* _notValidator()
{
   return new v::Map(
      "def", _baseValidator(),
      NULL);
}

static v::Validator* _notCompareValidator()
{
   return new v::Map(
      "def", new v::Map(
         "key1", new v::Type(String),
         "key2", new v::Type(String),
         NULL),
      NULL);
}

static v::Validator* _notValidValidator()
{
   return new v::Type(Map);
}

static v::Validator* _regexValidator()
{
   return new v::Map(
      "def", new v::Type(String),
      NULL);
}

static v::Validator* _typeValidator()
{
   return new v::Map(
      "def", new v::Any(
         new v::Equals("String"),
         new v::Equals("UInt32"),
         new v::Equals("Int32"),
         new v::Equals("UInt64"),
         new v::Equals("Int64"),
         new v::Equals("Boolean"),
         new v::Equals("Map"),
         new v::Equals("Array"),
         NULL),
      NULL);
}

static v::Validator* _validValidator()
{
   return new v::Type(Map);
}

static v::Validator* _nullValidator()
{
   return new v::Type(Map);
}

static v::ValidatorRef _createAll(
   v::ValidatorFactory* vf, DynamicObject& def)
{
   v::All* rval = new v::All(NULL);

   DynamicObjectIterator i = def["def"].getIterator();
   while(i->hasNext())
   {
      rval->addValidatorRef(vf->createValidator(i->next()));
   }

   return rval;
}

static v::ValidatorRef _createAny(
   v::ValidatorFactory* vf, DynamicObject& def)
{
   v::Any* rval = new v::Any(NULL);

   DynamicObjectIterator i = def["def"].getIterator();
   while(i->hasNext())
   {
      rval->addValidatorRef(vf->createValidator(i->next()));
   }

   return rval;
}

static v::ValidatorRef _createArray(
   v::ValidatorFactory* vf, DynamicObject& def)
{
   v::Array* rval = new v::Array();

   DynamicObjectIterator i = def["def"].getIterator();
   while(i->hasNext())
   {
      DynamicObject& next = i->next();
      int index = next->hasMember("index") ? next["index"]->getInt32() : -1;
      rval->addValidatorRef(index, vf->createValidator(next));
   }

   return rval;
}

static v::ValidatorRef _createCompare(
   v::ValidatorFactory* vf, DynamicObject& def)
{
   return new v::Compare(
      def["def"]["key1"], def["def"]["key2"],
      def->hasMember("error") ? def["error"]->getString() : NULL);
}

static v::ValidatorRef _createEach(
   v::ValidatorFactory* vf, DynamicObject& def)
{
   v::ValidatorRef vr = vf->createValidator(def["def"]);
   return new v::Each(vr);
}

static v::ValidatorRef _createEquals(
   v::ValidatorFactory* vf, DynamicObject& def)
{
   return new v::Equals(
      def["def"], def->hasMember("error") ? def["error"]->getString() : NULL);
}

static v::ValidatorRef _createIn(
   v::ValidatorFactory* vf, DynamicObject& def)
{
   return new v::In(
      def["def"], def->hasMember("error") ? def["error"]->getString() : NULL);
}

static v::ValidatorRef _createInt(
   v::ValidatorFactory* vf, DynamicObject& def)
{
   v::Int* rval = NULL;

   // FIXME: support other v::Int constructors

   const char* error =
      def->hasMember("error") ? def["error"]->getString() : NULL;
   if(def["def"]->hasMember("type"))
   {
      v::Int::IntegerType it = v::Int::Zero;
      const char* type = def["def"]["type"];
      if(strcmp(type, "Zero") == 0)
      {
         it = v::Int::Zero;
      }
      else if(strcmp(type, "Positive") == 0)
      {
         it = v::Int::Positive;
      }
      else if(strcmp(type, "Negative") == 0)
      {
         it = v::Int::Negative;
      }
      else if(strcmp(type, "NonPositive") == 0)
      {
         it = v::Int::NonPositive;
      }
      else if(strcmp(type, "NonNegative") == 0)
      {
         it = v::Int::NonNegative;
      }
      rval = new v::Int(it, error);
   }
   else
   {
      rval = new v::Int(def["def"]["min"], def["def"]["max"], error);
   }

   return rval;
}

static v::ValidatorRef _createMap(
   v::ValidatorFactory* vf, DynamicObject& def)
{
   v::Map* rval = new v::Map();

   // FIXME: might be a nice feature to automatically create v::Equals
   // validator if "next" is not a map, would shorten validator JSON

   DynamicObjectIterator i = def["def"].getIterator();
   while(i->hasNext())
   {
      DynamicObject& next = i->next();
      rval->addValidatorRef(i->getName(), vf->createValidator(next));
   }

   return rval;
}

static v::ValidatorRef _createMax(
   v::ValidatorFactory* vf, DynamicObject& def)
{
   return new v::Max(
      def["def"], def->hasMember("error") ? def["error"]->getString() : NULL);
}

static v::ValidatorRef _createMember(
   v::ValidatorFactory* vf, DynamicObject& def)
{
   return new v::Member(
      def["def"], def->hasMember("error") ? def["error"]->getString() : NULL);
}

static v::ValidatorRef _createMin(
   v::ValidatorFactory* vf, DynamicObject& def)
{
   return new v::Min(
      def["def"], def->hasMember("error") ? def["error"]->getString() : NULL);
}

static v::ValidatorRef _createNot(
   v::ValidatorFactory* vf, DynamicObject& def)
{
   v::ValidatorRef vr = vf->createValidator(def["def"]);
   return new v::Not(
      vr, def->hasMember("error") ? def["error"]->getString() : NULL);
}

static v::ValidatorRef _createNotCompare(
   v::ValidatorFactory* vf, DynamicObject& def)
{
   return new v::NotCompare(
      def["def"]["key1"], def["def"]["key2"],
      def->hasMember("error") ? def["error"]->getString() : NULL);
}

static v::ValidatorRef _createNotValid(
   v::ValidatorFactory* vf, DynamicObject& def)
{
   return new v::NotValid(
      def->hasMember("error") ? def["error"]->getString() : NULL);
}

static v::ValidatorRef _createRegex(
   v::ValidatorFactory* vf, DynamicObject& def)
{
   return new v::Regex(
      def["def"], def->hasMember("error") ? def["error"]->getString() : NULL);
}

static v::ValidatorRef _createType(
   v::ValidatorFactory* vf, DynamicObject& def)
{
   DynamicObjectType type = monarch::rt::String;
   const char* t = def["def"];
   if(strcmp(t, "String") == 0)
   {
      type = monarch::rt::String;
   }
   else if(strcmp(t, "UInt32") == 0)
   {
      type = monarch::rt::UInt32;
   }
   else if(strcmp(t, "Int32") == 0)
   {
      type = monarch::rt::Int32;
   }
   else if(strcmp(t, "UInt64") == 0)
   {
      type = monarch::rt::UInt64;
   }
   else if(strcmp(t, "Int64") == 0)
   {
      type = monarch::rt::Int64;
   }
   else if(strcmp(t, "Boolean") == 0)
   {
      type = monarch::rt::Boolean;
   }
   else if(strcmp(t, "Map") == 0)
   {
      type = monarch::rt::Map;
   }
   else if(strcmp(t, "Array") == 0)
   {
      type = monarch::rt::Array;
   }

   return new v::Type(
      type, def->hasMember("error") ? def["error"]->getString() : NULL);
}

static v::ValidatorRef _createValid(
   v::ValidatorFactory* vf, DynamicObject& def)
{
   return new v::Valid();
}

static v::ValidatorRef _createNull(
   v::ValidatorFactory* vf, DynamicObject& def)
{
   return new v::Null(
      def->hasMember("error") ? def["error"]->getString() : NULL);
}

static void _createValidatorDef(
   v::ValidatorFactory::ValidatorDefMap& vdm, const char* type,
   v::Validator* validator, v::ValidatorFactory::CreateValidatorFunc function)
{
   v::ValidatorFactory::ValidatorDef def;
   def.validator = validator;
   def.function = function;
   vdm[type] = def;
}

v::ValidatorFactory::ValidatorFactory(bool sync) :
   mLoadLock(NULL)
{
   if(sync)
   {
      mLoadLock = new SharedLock();
   }

   // create definition validators
   _createValidatorDef(
      mValidatorDefs, MO_VALIDATOR_ALL, _allValidator(), &_createAll);
   _createValidatorDef(
      mValidatorDefs, MO_VALIDATOR_ANY, _anyValidator(), &_createAny);
   _createValidatorDef(
      mValidatorDefs, MO_VALIDATOR_ARRAY, _arrayValidator(), &_createArray);
   _createValidatorDef(
      mValidatorDefs, MO_VALIDATOR_COMPARE, _compareValidator(), &_createCompare);
   _createValidatorDef(
      mValidatorDefs, MO_VALIDATOR_EACH, _eachValidator(), &_createEach);
   _createValidatorDef(
      mValidatorDefs, MO_VALIDATOR_EQUALS, _equalsValidator(), &_createEquals);
   _createValidatorDef(
      mValidatorDefs, MO_VALIDATOR_IN, _inValidator(), &_createIn);
   _createValidatorDef(
      mValidatorDefs, MO_VALIDATOR_INT, _intValidator(), &_createInt);
   _createValidatorDef(
      mValidatorDefs, MO_VALIDATOR_MAP, _mapValidator(), &_createMap);
   _createValidatorDef(
      mValidatorDefs, MO_VALIDATOR_MAX, _maxValidator(), &_createMax);
   _createValidatorDef(
      mValidatorDefs, MO_VALIDATOR_MEMBER, _memberValidator(), &_createMember);
   _createValidatorDef(
      mValidatorDefs, MO_VALIDATOR_MIN, _minValidator(), &_createMin);
   _createValidatorDef(
      mValidatorDefs, MO_VALIDATOR_NOT, _notValidator(), &_createNot);
   _createValidatorDef(
      mValidatorDefs, MO_VALIDATOR_NOT_COMPARE,
      _notCompareValidator(), &_createNotCompare);
   _createValidatorDef(
      mValidatorDefs, MO_VALIDATOR_NOT_VALID,
      _notValidValidator(), &_createNotValid);
   _createValidatorDef(
      mValidatorDefs, MO_VALIDATOR_REGEX, _regexValidator(), &_createRegex);
   _createValidatorDef(
      mValidatorDefs, MO_VALIDATOR_TYPE, _typeValidator(), &_createType);
   _createValidatorDef(
      mValidatorDefs, MO_VALIDATOR_VALID, _validValidator(), &_createValid);
   _createValidatorDef(
      mValidatorDefs, MO_VALIDATOR_NULL, _nullValidator(), &_createNull);
}

v::ValidatorFactory::~ValidatorFactory()
{
   // clean up types
   for(ValidatorMap::iterator i = mValidators.begin(); i != mValidators.end();
       ++i)
   {
      free(const_cast<char*>(i->first));
   }

   free(mLoadLock);
}

bool v::ValidatorFactory::loadValidatorDefinitions(const char* path)
{
   bool rval = true;

   // get a list of all of the definition files to add
   FileList fileList;

   // split path by path separator
   DynamicObject paths = StringTools::split(path, File::PATH_SEPARATOR);
   {
      DynamicObjectIterator i = paths.getIterator();
      while(i->hasNext())
      {
         File file(i->next());
         file->listFiles(fileList);
      }
   }

   // get custom top-level validator
   ValidatorRef custom = _customValidator(true);

   // create a map to store validator definitions
   DynamicObject defs;
   defs->setType(monarch::rt::Map);

   // FIXME: this could be reworked to read the JSON outside of the lock

   // handle sync if necessary
   if(mLoadLock != NULL)
   {
      mLoadLock->lockExclusive();
   }

   // start loading definitions
   JsonReader reader;
   IteratorRef<File> i = fileList->getIterator();
   while(rval && i->hasNext())
   {
      File& file = i->next();
      if(file->isFile())
      {
         // read definition from json
         DynamicObject def;
         FileInputStream fis(file);
         rval =
            reader.start(def) && reader.read(&fis) && reader.finish() &&
            custom->isValid(def);
         if(rval)
         {
            // ensure type is not a duplicate
            const char* type = def["type"];
            ValidatorMap::iterator vmi = mValidators.find(type);
            if(vmi != mValidators.end() || defs->hasMember(type))
            {
               ExceptionRef e = new Exception(
                  "Could not define Validator. "
                  "Duplicate Validator type detected.",
                  VF_EXCEPTION ".DuplicateType");
               e->getDetails()["type"] = type;
               Exception::set(e);
               rval = false;
            }
            else
            {
               // add to definitions map
               defs[type] = def;
            }
         }
      }
   }

   // define validators from the definitions map
   rval = rval && defineValidators(defs);

   // handle sync if necessary
   if(mLoadLock != NULL)
   {
      mLoadLock->unlockExclusive();
   }

   if(!rval)
   {
      ExceptionRef e = new Exception(
         "Could not load Validator definition(s).",
         VF_EXCEPTION ".DefinitionError");
      e->getDetails()["path"] = path;
      Exception::push(e);
   }

   return rval;
}

bool v::ValidatorFactory::loadValidatorDefinition(InputStream* is)
{
   bool rval = true;

   // get custom top-level validator
   ValidatorRef custom = _customValidator(true);

   // read definition from json
   DynamicObject def;
   JsonReader reader;
   rval =
      reader.start(def) && reader.read(is) && reader.finish() &&
      custom->isValid(def);
   if(rval)
   {
      // handle sync if necessary
      if(mLoadLock != NULL)
      {
         mLoadLock->lockExclusive();
      }

      // ensure type is not a duplicate
      const char* type = def["type"];
      ValidatorMap::iterator vmi = mValidators.find(type);
      if(vmi != mValidators.end())
      {
         ExceptionRef e = new Exception(
            "Could not define Validator. Duplicate Validator type detected.",
            VF_EXCEPTION ".DuplicateType");
         e->getDetails()["type"] = type;
         Exception::set(e);
         rval = false;
      }
      else
      {
         // add to definitions map
         DynamicObject defs;
         defs[type] = def;

         // define validator
         rval = defineValidators(defs);
      }

      // handle sync if necessary
      if(mLoadLock != NULL)
      {
         mLoadLock->unlockExclusive();
      }
   }

   if(!rval)
   {
      ExceptionRef e = new Exception(
         "Could not load Validator definition.",
         VF_EXCEPTION ".DefinitionError");
      Exception::push(e);
   }

   return rval;
}

bool v::ValidatorFactory::loadValidatorDefinition(DynamicObject& def)
{
   bool rval = true;

   // get custom top-level validator
   ValidatorRef custom = _customValidator(true);
   rval = custom->isValid(def);
   if(rval)
   {
      // handle sync if necessary
      if(mLoadLock != NULL)
      {
         mLoadLock->lockExclusive();
      }

      // ensure type is not a duplicate
      const char* type = def["type"];
      ValidatorMap::iterator vmi = mValidators.find(type);
      if(vmi != mValidators.end())
      {
         ExceptionRef e = new Exception(
            "Could not define Validator. Duplicate Validator type detected.",
            VF_EXCEPTION ".DuplicateType");
         e->getDetails()["type"] = type;
         Exception::set(e);
         rval = false;
      }
      else
      {
         // add to definitions map, clone it to prevent changes
         DynamicObject defs;
         defs[type] = def.clone();

         // define validator
         rval = defineValidators(defs);
      }

      // handle sync if necessary
      if(mLoadLock != NULL)
      {
         mLoadLock->unlockExclusive();
      }
   }

   if(!rval)
   {
      ExceptionRef e = new Exception(
         "Could not load Validator definition.",
         VF_EXCEPTION ".DefinitionError");
      Exception::push(e);
   }

   return rval;
}

v::ValidatorRef v::ValidatorFactory::createValidator(const char* type)
{
   ValidatorRef rval(NULL);

   // handle sync if necessary
   if(mLoadLock != NULL)
   {
      mLoadLock->unlockShared();
   }

   ValidatorMap::iterator i = mValidators.find(type);
   if(i == mValidators.end())
   {
      ExceptionRef e = new Exception(
         "Could not create Validator. Validator type not found.",
         VF_EXCEPTION ".NotFound");
      Exception::set(e);
   }
   else
   {
      rval = i->second;
   }

   // handle sync if necessary
   if(mLoadLock != NULL)
   {
      mLoadLock->unlockShared();
   }

   return rval;
}

v::ValidatorRef v::ValidatorFactory::createValidator(DynamicObject& def)
{
   ValidatorRef rval(NULL);

   // create new custom validator
   if(def->hasMember("extends"))
   {
      // must be a built-in type
      rval = mValidatorDefs.find(def["extends"])->second.function(this, def);
   }
   else
   {
      // try to get a built-in validator
      ValidatorDefMap::iterator i = mValidatorDefs.find(def["type"]);
      if(i != mValidatorDefs.end())
      {
         rval = i->second.function(this, def);
      }
      else
      {
         // use already-created validator (all previously defined custom
         // validators will be cached)
         rval = mValidators.find(def["type"])->second;
      }
   }

   // if def is optional, wrap in optional validator
   if(def->hasMember("optional") && def["optional"]->getBoolean())
   {
      ValidatorRef optional = new v::Optional(rval);
      rval = optional;
   }

   return rval;
}

v::ValidatorRef v::ValidatorFactory::getDefValidator(const char* type)
{
   ValidatorRef rval(NULL);

   ValidatorDefMap::iterator i = mValidatorDefs.find(type);
   if(i != mValidatorDefs.end())
   {
      rval = i->second.validator;
   }

   return rval;
}

bool v::ValidatorFactory::defineValidators(DynamicObject& defs)
{
   bool rval = true;

   DynamicObjectIterator i = defs.getIterator();
   while(rval && i->hasNext())
   {
      DynamicObject& def = i->next();

      // validate the definition
      ValidatorRef extends = getDefValidator(def["extends"]);
      rval = extends->isValid(def);
      if(rval)
      {
         // replace type with extends in order to do recursive validation
         def["type"] = def["extends"];
         def->removeMember("extends");
      }

      // do recursive validation and gather dependencies
      def["deps"]->setType(monarch::rt::Array);
      rval = rval && recursiveValidate(def, def["deps"]);
   }

   // create a list of created validators to remove if there is a failure
   DynamicObject created;
   created->setType(monarch::rt::Array);

   /* This loop will iterate over all of the new definitions checking
    * dependencies and creating validators. Once the dependencies for a
    * particular validator have been met, it will be created. If the loop
    * goes through all of the definitions and not a single validator was
    * created, then an exception will be set because a dependency was not met.
    */
   bool unmet = false;
   while(rval && !unmet && defs->length() > 0)
   {
      unmet = true;
      i = defs.getIterator();
      while(i->hasNext())
      {
         DynamicObject& def = i->next();

         // check all custom dependencies
         bool met = true;
         DynamicObjectIterator di = def["deps"].getIterator();
         while(met && di->hasNext())
         {
            const char* dep = di->next();
            if(mValidators.find(dep) == mValidators.end())
            {
               // dependency not yet met
               met = false;
            }
         }

         if(met)
         {
            // restore extends and type
            def["extends"] = def["type"].clone();
            def["type"] = i->getName();

            // create validator
            ValidatorRef vr = createValidator(def);
            mValidators[strdup(i->getName())] = vr;
            created->append(i->getName());
            i->remove();
            unmet = false;
         }
      }
   }

   if(rval && unmet)
   {
      ExceptionRef e = new Exception(
         "Could not define custom Validator(s). Dependencies were not met.",
         VF_EXCEPTION ".DependencyNotMet");
      e->getDetails()["notDefined"] = defs;
      Exception::set(e);
      rval = false;
   }

   // failure, remove any created validators
   if(!rval)
   {
      i = created.getIterator();
      while(i->hasNext())
      {
         ValidatorMap::iterator vmi = mValidators.find(i->next());
         free(const_cast<char*>(vmi->first));
         mValidators.erase(vmi);
      }
   }

   return rval;
}

bool v::ValidatorFactory::recursiveValidate(
   DynamicObject& def, DynamicObject& deps)
{
   bool rval = true;

   // get definition validator
   const char* type = def["type"];
   ValidatorRef validator = getDefValidator(def["type"]);
   if(validator.isNull())
   {
      // use non-top level custom validator
      validator = _customValidator(false);
   }

   // validate definition
   rval = validator->isValid(def);
   if(rval && def->hasMember("def"))
   {
      // recurse for container validator types
      if(strcmp(type, MO_VALIDATOR_ALL) == 0 ||
         strcmp(type, MO_VALIDATOR_ANY) == 0 ||
         strcmp(type, MO_VALIDATOR_ARRAY) == 0 ||
         strcmp(type, MO_VALIDATOR_MAP) == 0)
      {
         DynamicObjectIterator i = def["def"].getIterator();
         while(rval && i->hasNext())
         {
            rval = recursiveValidate(i->next(), deps);
         }
      }
      else if(strcmp(type, MO_VALIDATOR_EACH) == 0)
      {
         rval = recursiveValidate(def["def"], deps);
      }
   }

   if(rval)
   {
      // add custom dependency if type is not a built in
      ValidatorDefMap::iterator i = mValidatorDefs.find(type);
      if(i == mValidatorDefs.end())
      {
         // add uniquely
         if(deps->indexOf(type) == -1)
         {
            deps->append(type);
         }
      }
   }

   return rval;
}
