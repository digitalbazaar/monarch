/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/validation/ValidatorContext.h"

using namespace db::rt;
using namespace db::validation;

ValidatorContext::ValidatorContext(DynamicObject* state)
{
   mPath = NULL;
   mState = state;
   mSetExceptions = true;
}

ValidatorContext::~ValidatorContext()
{
   if(mPath != NULL)
   {
      for(std::vector<char*>::iterator i = mPath->begin();
          i != mPath->end(); i++)
      {
         free(*i);
      }
      
      delete mPath;
   }
}

db::rt::DynamicObject& ValidatorContext::getState()
{
   return *mState;
}

bool ValidatorContext::setExceptions(bool set)
{
   bool rval = mSetExceptions;
   mSetExceptions = set;
   return rval;
}

void ValidatorContext::pushPath(const char* path)
{
   if(mPath == NULL)
   {
      mPath = new std::vector<char*>;
   }
   mPath->push_back(strdup(path));
   
   printf("ValidatorContext::pushPath: '%s'\n", path);
}

void ValidatorContext::popPath()
{
   if(mPath != NULL)
   {
      char* path = mPath->back(); 
      mPath->pop_back();
      free(path);
   }
}

unsigned int ValidatorContext::getDepth()
{
   unsigned int rval = 0;
   if(mPath != NULL)
   {
      rval = mPath->size();
   }
   return rval;
}

std::string ValidatorContext::getPath()
{
   std::string rval;
   
   if(mPath != NULL)
   {
      for(std::vector<char*>::iterator i = mPath->begin();
         i != mPath->end();
         i++)
      {
         rval.append(*i);
      }
   }
   
   return rval;
}

DynamicObject ValidatorContext::addError(const char* type)
{
   DynamicObject errorDetail;
   
   // Skip setting exceptions if requested.  Return errorDetail regardless.
   if(mSetExceptions)
   {
      ExceptionRef e;
      
      if(!Exception::hasLast())
      {
         e = new Exception(
            "Object not valid.", "db.validation.ValidationError");
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
      std::string fullpath = getPath();
      e->getDetails()["errors"][fullpath.c_str()] = errorDetail;
   }
   
   return errorDetail;
}
