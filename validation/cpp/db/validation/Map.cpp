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
   DynamicObject* state,
   std::vector<const char*>* path)
{
   bool rval = true;
   bool madePath = false;
   
   // create a path if there isn't one yet
   if(!path)
   {
      madePath = true;
      path = new std::vector<const char*>;
   }

   std::vector<std::pair<const char*,Validator*> >::iterator i;
   
   for(i = mValidators.begin();
      i != mValidators.end();
      i++)
   {
      if(obj->hasMember(i->first))
      {
         // only add a "." if this is not a root map
         if(path->size() != 0)
         {
            path->push_back(".");
         }
         path->push_back(i->first);
         rval &= i->second->isValid(obj[i->first], state, path);
         path->pop_back();
         if(path->size() == 1)
         {
            path->pop_back();
         }
      }
      else if(!i->second->isOptional(state))
      {
         rval = false;
         DynamicObject detail = addError(path, "db.validation.MissingKey");
         detail["key"] = i->first;
      }
   }

   if(madePath)
   {
      delete path;
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
