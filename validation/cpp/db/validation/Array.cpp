/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/validation/Array.h"

using namespace db::rt;
using namespace db::validation;

Array::Array()
{
}

Array::Array(int index, ...)
{
   va_list ap;
   
   va_start(ap, index);
   addValidators(index, ap);
   va_end(ap);
}
   
Array::~Array()
{
   std::vector<std::pair<int,Validator*> >::iterator i;
   
   for(i = mValidators.begin();
      i != mValidators.end();
      i++)
   {
      delete i->second;
   }
}
   
bool Array::isValid(
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

   std::vector<std::pair<int,Validator*> >::iterator i;
   
   for(i = mValidators.begin();
      i != mValidators.end();
      i++)
   {
      if(obj->length() >= i->first)
      {
         // add [#] indexing to path even if at root
         char idx[23];
         snprintf(idx, 23, "[%d]", i->first);
         path->push_back(idx);
         rval &= i->second->isValid(obj[i->first], state, path);
         path->pop_back();
      }
      else
      {
         rval = false;
         DynamicObject detail = addError(path, "db.validation.MissingIndex");
         detail["index"] = i->first;
      }
   }

   if(madePath)
   {
      delete path;
   }
   
   return rval;
}

void Array::addValidator(int index, Validator* validator)
{
   mValidators.push_back(std::make_pair(index, validator));
}

void Array::addValidators(int index, va_list ap)
{
   while(index != -1)
   {
      Validator* v = va_arg(ap, Validator*);
      addValidator(index, v);
      index = va_arg(ap, int);
   }
}

void Array::addValidators(int index, ...)
{
   va_list ap;
   
   va_start(ap, index);
   addValidators(index, ap);
   va_end(ap);
}
