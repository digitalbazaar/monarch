/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/validation/Map.h"

using namespace db::rt;
using namespace db::validation;

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
   std::vector<std::pair<const char*,Validator*> >::iterator i;
   
   for(i = mValidators.begin();
      i != mValidators.end();
      i++)
   {
      delete i->second;
   }
}

bool Map::isValid(
   DynamicObject& obj,
   ValidatorContext* context)
{
   bool rval = true;
   
   std::vector<std::pair<const char*,Validator*> >::iterator i;
   
   for(i = mValidators.begin();
      i != mValidators.end();
      i++)
   {
      if(obj->hasMember(i->first))
      {
         // only add a "." if this is not a root map
         if(context->getDepth() != 0)
         {
            context->pushPath(".");
         }
         context->pushPath(i->first);
         rval = rval && i->second->isValid(obj[i->first], context);
	 context->popPath();
         if(context->getDepth() == 1)
         {
            context->popPath();
         }
      }
      else if(!i->second->isOptional(context))
      {
         rval = false;
         DynamicObject detail = context->addError("db.validation.MissingKey");
         detail["key"] = i->first;
      }
   }

   return rval;
}

void Map::addValidator(const char* key, Validator* validator)
{
   mValidators.push_back(std::make_pair(key, validator));
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
