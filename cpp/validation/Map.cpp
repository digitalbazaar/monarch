/*
 * Copyright (c) 2008-2011 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/validation/Map.h"

using namespace monarch::rt;
using namespace monarch::validation;

Map::Map()
{
}

Map::Map(const char* key, ...)
{
   va_list ap;

   va_start(ap, key);
   addValidators(key, ap);
   va_end(ap);
}

Map::~Map()
{
   for(Validators::iterator i = mValidators.begin();
       i != mValidators.end(); ++i)
   {
      // clean up key
      free(const_cast<char*>(i->first));

      // only clean up validator if it has no reference
      if(i->second.reference.isNull())
      {
         delete i->second.validator;
      }
   }
}

bool Map::isValid(
   DynamicObject& obj,
   ValidatorContext* context)
{
   bool rval = true;

   if(!obj.isNull() && obj->getType() == monarch::rt::Map)
   {
      Validators::iterator i;
      for(i = mValidators.begin(); i != mValidators.end(); ++i)
      {
         // only add a "." if this is not a root map
         if(context->getDepth() != 0)
         {
            context->pushPath(".");
         }
         context->pushPath(i->first);
         if(obj->hasMember(i->first))
         {
            // do not short-circuit to ensure all keys tested
            if(!i->second.validator->isValid(obj[i->first], context))
            {
               rval = false;
            }
         }
         else if(!i->second.validator->isOptional(context))
         {
            rval = false;
            DynamicObject detail =
               context->addError("monarch.validation.MissingField", &obj);
            detail["validator"] = "monarch.validator.Map";
            detail["message"] = "A required field has not been specified.";
            detail["key"] = i->first;
         }
         context->popPath();
         if(context->getDepth() > 0)
         {
            context->popPath();
         }
      }
   }
   else
   {
      rval = false;
      DynamicObject detail =
         context->addError("monarch.validation.TypeError", &obj);
      detail["validator"] = "monarch.validator.Map";
      detail["message"] = "The given object type must a mapping (Map) type";
   }

   return rval;
}

size_t Map::length()
{
   return mValidators.size();
}

void Map::addValidator(const char* key, Validator* validator)
{
   Entry e = {&(*validator), NULL};
   mValidators.push_back(std::make_pair(strdup(key), e));
}

void Map::addValidatorRef(const char* key, ValidatorRef validator)
{
   Entry e = {&(*validator), validator};
   mValidators.push_back(std::make_pair(strdup(key), e));
}

void Map::addValidators(const char* key, va_list ap)
{
   while(key != NULL)
   {
      Validator* v = va_arg(ap, Validator*);
      addValidator(key, v);
      key = va_arg(ap, const char*);
   }
}

void Map::addValidators(const char* key, ...)
{
   va_list ap;

   va_start(ap, key);
   addValidators(key, ap);
   va_end(ap);
}
