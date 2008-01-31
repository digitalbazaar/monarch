/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */

#include "db/logging/Category.h"

using namespace db::logging;
 
// DO NOT INITIALIZE THESE VARIABLES!
// These are not initialized on purpose due to initialization code issues.
Category* DB_DEFAULT_CAT;
Category* DB_ALL_CAT;

Category::Category(const char* id, const char* name, const char* description) :
   mId(NULL),
   mName(NULL),
   mDescription(NULL)
{
   setId(id);
   setName(name);
   setDescription(description);
}
   
Category::~Category()
{
   setId(NULL);
   setName(NULL);
   setDescription(NULL);
}

void Category::initialize()
{
   DB_DEFAULT_CAT = new Category(
      "DB_DEFAULT",
      "Default",
      "Default category for general use");
   DB_ALL_CAT = new Category(
      NULL,
      NULL,
      "Pseudo-category that matches ALL other categories");
}

void Category::cleanup()
{
   delete DB_DEFAULT_CAT;
   DB_DEFAULT_CAT = NULL;
   
   delete DB_ALL_CAT;
   DB_ALL_CAT = NULL;
}
   
void Category::setId(const char* id)
{
   if(mId)
   {
      free(mId);
   }
   mId = id ? strdup(id) : NULL;
}

const char* Category::getId()
{
   return mId;
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
