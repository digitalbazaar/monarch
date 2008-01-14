/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */

#include "db/logging/Category.h"
#include "db/logging/LoggingCategories.h"

using namespace db::logging;
 
// DO NOT INITIALIZE THESE VARIABLES!
// These are not initialized on purpose due to initialization code issues.
Category* DB_DEFAULT_CAT;
Category* DB_ALL_CAT;

Category::Category(const char* name, const char* shortName,
   const char* description) :
   mName(NULL),
   mShortName(NULL),
   mDescription(NULL)
{
   setName(name);
   setShortName(shortName);
   setDescription(description);
}
   
Category::~Category()
{
   setName(NULL);
   setShortName(NULL);
   setDescription(NULL);
}

void Category::initialize()
{
   DB_DEFAULT_CAT = new Category(
      "Default",
      "DB_DEFAULT",
      "Default category for general use");
   DB_ALL_CAT = new Category(
      NULL,
      NULL,
      "Pseudo-category that matches ALL other categories");
   
   LoggingCategories::initialize();
}

void Category::cleanup()
{
   LoggingCategories::cleanup();

   delete DB_DEFAULT_CAT;
   DB_DEFAULT_CAT = NULL;
   
   delete DB_ALL_CAT;
   DB_ALL_CAT = NULL;
}
   
void Category::setName(const char* name)
{
   if(mName)
   {
      free(mName);
   }
   mName = name ? strdup(name) : NULL;
}

const char* Category::getName()
{
   return mName ? mName : "<?>";
}

void Category::setShortName(const char* shortName)
{
   if(mShortName)
   {
      free(mShortName);
   }
   mShortName = shortName ? strdup(shortName) : NULL;
}

const char* Category::getShortName()
{
   return mShortName;
}

void Category::setDescription(const char* description)
{
   if(mDescription)
   {
      free(mDescription);
   }
   mDescription = description ? strdup(description) : NULL;
}

const char* Category::getDescription()
{
   return mDescription;
}
