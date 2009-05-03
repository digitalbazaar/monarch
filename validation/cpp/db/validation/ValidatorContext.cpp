/*
 * Copyright (c) 2008-2009 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/rt/Exception.h"
#include "db/validation/ValidatorContext.h"

#include <cstdlib>

using namespace db::rt;
using namespace db::validation;

ValidatorContext::ValidatorContext(DynamicObject* state) :
   mResults(NULL)
{
   mPath = NULL;
   mState = state;
   mSetExceptions = true;
   clearResults();
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

void ValidatorContext::addSuccess()
{
   mResults["successes"] = mResults["successes"]->getUInt32() + 1;
}

DynamicObject ValidatorContext::addError(
   const char* type, DynamicObject* object)
{
   DynamicObject errorDetail;
   
   // setup error detail
   errorDetail["type"] = type;
   // FIXME: localize message -- lehn
   // FIXME: really? do we need to mention this, because we'd have to
   //        do this for every string in the system.. -- manu
   errorDetail["message"] = "The given value does not meet all of the data "
      "validation requirements. Please examine the error details for more "
      "information about the specific requirements.";
   if(object != NULL)
   {
      errorDetail["invalidValue"] = *object;
   }
   
   // add error detail to results errors
   mResults["errors"]->append(errorDetail);
   
   // Skip setting exceptions if requested. Return errorDetail regardless.
   if(mSetExceptions)
   {
      ExceptionRef e;
      
      if(!Exception::hasLast())
      {
         e = new Exception(
            "The given object does not meet all of the data validation "
            "requirements. Please examine the error details for more "
            "information about the specific requirements.", 
            "db.validation.ValidationError");
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
               "The given object does not meet all of the data validation "
               "requirements. Please examine the error details for more "
               "information about the specific requirements.",
               "db.validation.ValidationError");
            Exception::setLast(e, true);
         }
      }
      
      // add detail to "errors" section of exception details
      std::string fullpath = getPath();
      e->getDetails()["errors"][fullpath.c_str()] = errorDetail;
   }
   
   return errorDetail;
}

DynamicObject ValidatorContext::getResults()
{
   return mResults;
}

void ValidatorContext::clearResults()
{
   mResults = DynamicObject();
   mResults["successes"] = 0;
   mResults["errors"]->setType(Array);
   mResults["errors"]->clear();
}
