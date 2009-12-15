/*
 * Copyright (c) 2008-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/logging/Category.h"

#include <cstdlib>
#include <cstring>

using namespace monarch::logging;

// DO NOT INITIALIZE THESE VARIABLES!
// These are not initialized on purpose due to initialization code issues.
Category* MO_DEFAULT_CAT;
Category* MO_ALL_CAT;

Category::Category(const char* id, const char* name, const char* description) :
   mId(NULL),
   mName(NULL),
   mDescription(NULL),
   mAnsiEscapeCodes(NULL)
{
   setId(id);
   setName(name);
   setDescription(description);
}

Category::~Category()
{
   Category::setId(NULL);
   Category::setName(NULL);
   Category::setDescription(NULL);
   Category::setAnsiEscapeCodes(NULL);
}

void Category::initialize()
{
   MO_DEFAULT_CAT = new Category(
      "MO_DEFAULT",
      "Default",
      "Default category for general use");
   MO_ALL_CAT = new Category(
      NULL,
      NULL,
      "Pseudo-category that matches ALL other categories");
}

void Category::cleanup()
{
   delete MO_DEFAULT_CAT;
   MO_DEFAULT_CAT = NULL;

   delete MO_ALL_CAT;
   MO_ALL_CAT = NULL;
}

void Category::setId(const char* id)
{
   if(mId != NULL)
   {
      free(mId);
   }
   mId = (id != NULL ? strdup(id) : NULL);
}

const char* Category::getId()
{
   return mId;
}

void Category::setName(const char* name)
{
   if(mName != NULL)
   {
      free(mName);
   }
   mName = (name != NULL ? strdup(name) : NULL);
}

const char* Category::getName()
{
   return mName != NULL ? mName : "<?>";
}

void Category::setDescription(const char* description)
{
   if(mDescription != NULL)
   {
      free(mDescription);
   }
   mDescription = (description != NULL ? strdup(description) : NULL);
}

const char* Category::getDescription()
{
   return mDescription;
}

void Category::setAnsiEscapeCodes(const char* ansi)
{
   if(mAnsiEscapeCodes != NULL)
   {
      free(mAnsiEscapeCodes);
   }
   mAnsiEscapeCodes = (ansi != NULL ? strdup(ansi) : NULL);
}

const char* Category::getAnsiEscapeCodes()
{
   return mAnsiEscapeCodes != NULL ? mAnsiEscapeCodes : "";
}
