/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/validation/Validator.h"

using namespace db::rt;
using namespace db::validation;

Validator::Validator()
{
}

Validator::~Validator()
{
}

std::string Validator::stringFromPath(std::vector<const char*>* path)
{
   std::string rval;
   
   if(!path)
   {
      rval = "";
   }
   else
   {
      for(std::vector<const char*>::iterator i = path->begin();
         i != path->end();
         i++)
      {
         rval.append(*i);
      }
   }
   
   return rval;
}

DynamicObject Validator::addError(
   std::vector<const char*>* path,
   const char* type)
{
   DynamicObject errorDetail;
   
   ExceptionRef e;
   
   if(!Exception::hasLast())
   {
      e = new Exception("Object not valid.", "db.validation.ValidationError");
      Exception::setLast(e, false);
   }
   else
   {
      e = Exception::getLast();
      // Check we are adding to a ValidationError
      if(strcmp(e->getType(), "db.validation.ValidationError") != 0)
      {
         // FIXME: this is a bit bogus.  If validation checking keeps causing
         // other exceptions then a long cause chain could be generated
         // switching between ValidationError and other types.
         e = new Exception(
            "Object not valid.", "db.validation.ValidationError");
         Exception::setLast(e, true);
      }
   }

   // setup error detail   
   errorDetail["type"] = type;
   // FIXME: localize message
   errorDetail["message"] = "Invalid value!";
   
   // add detail to "errors" section of exception details
   std::string fullpath = stringFromPath(path);
   e->getDetails()["errors"][fullpath.c_str()] = errorDetail;
   
   return errorDetail;
}
