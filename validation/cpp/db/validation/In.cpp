/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/validation/In.h"
#include "db/rt/DynamicObjectIterator.h"

using namespace db::rt;
using namespace db::validation;

In::In(DynamicObject& contents) :
   mContents(contents)
{
}

In::~In()
{
}

bool In::isValid(
   DynamicObject& obj,
   DynamicObject* state,
   std::vector<const char*>* path)
{
   bool rval = false;
   bool madePath = false;
   
   // create a path if there isn't one yet
   if(!path)
   {
      madePath = true;
      path = new std::vector<const char*>;
   }

   switch(mContents->getType())
   {
      case Array:
         {
            DynamicObjectIterator doi = mContents.getIterator();
            while(!rval && doi->hasNext())
            {
               DynamicObject& member = doi->next();
               rval = obj == member;
            }
         }
         break;
      case Map:
         rval =
            obj->getType() == String &&
            mContents->hasMember(obj->getString());
         break;
      default:
         rval = false;
         DynamicObject detail = addError(path, "db.validation.InternalError");
         detail["message"] = "Invalid validator!";
         break;
   }
   
   if(!rval)
   {
      DynamicObject detail = addError(path, "db.validation.NotFound");
   }

   if(madePath)
   {
      delete path;
   }
   
   return rval;
}
