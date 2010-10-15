/*
 * Copyright (c) 2010 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/data/json/JsonLd.h"

#include "monarch/rt/DynamicObjectIterator.h"
#include "monarch/rt/Exception.h"

#include <cstdio>

using namespace std;
using namespace monarch::data;
using namespace monarch::data::json;
using namespace monarch::rt;

JsonLd::JsonLd()
{
}

JsonLd::~JsonLd()
{
}

/**
 * Normalizes a string using the given context.
 *
 * @param context the context.
 * @param str the string to normalize.
 * @param tmp to store the normalized string, may be realloc'd.
 *
 * @return a pointer to the normalized string.
 */
static const char* _normalizeString(
   DynamicObject& context, const char* str, char* tmp)
{
   const char* rval = str;

   // try to find a colon
   const char* ptr = strstr(str, ":");
   if(ptr != NULL)
   {
      // try to find prefix in string
      DynamicObjectIterator i = context.getIterator();
      while(rval == str && i->hasNext())
      {
         DynamicObject& uri = i->next();
         if(strncmp(str, i->getName(), ptr - str) == 0)
         {
            // prefix found, normalize string
            size_t len = strlen(uri->getString()) + strlen(ptr + 1) + 1;
            if(tmp == NULL || sizeof(tmp) < len)
            {
               tmp = (char*)realloc(tmp, len);
            }
            snprintf(tmp, len, "%s%s", uri->getString(), ptr + 1);
            rval = tmp;
         }
      }
   }

   return rval;
}

/**
 * Recursively normalizes the given input object.
 *
 * @param context the context to use (changes during recursion as necessary).
 * @param in the input object.
 * @param out the normalized output object.
 */
static void _normalize(
   DynamicObject& context, DynamicObject& in, DynamicObject& out)
{
   if(in.isNull())
   {
      out.setNull();
   }
   else
   {
      // initialize output
      DynamicObjectType inType = in->getType();
      out->setType(inType);

      // update context
      if(inType == Map && in->hasMember("#"))
      {
         context = in["#"];
      }

      if(inType == Map)
      {
         // normalize each non-context property in the map
         char* tmp = NULL;
         DynamicObjectIterator i = in.getIterator();
         while(i->hasNext())
         {
            DynamicObject& next = i->next();
            if(strcmp(i->getName(), "#") != 0)
            {
               // normalize key, normalize object
               _normalize(
                  context,
                  next, out[_normalizeString(context, i->getName(), tmp)]);
            }
         }
         if(tmp != NULL)
         {
            free(tmp);
         }
      }
      else if(inType == Array)
      {
         // normalize each object in the array
         DynamicObjectIterator i = in.getIterator();
         while(i->hasNext())
         {
            DynamicObject& next = i->next();
            _normalize(context, next, out->append());
         }
      }
      // only strings need normalization, numbers & booleans don't
      else if(inType == String)
      {
         // normalize string
         char* tmp = NULL;
         out = _normalizeString(context, in->getString(), tmp);
         if(tmp != NULL)
         {
            free(tmp);
         }
      }
   }
}

bool JsonLd::normalize(DynamicObject& in, DynamicObject& out)
{
   bool rval = true;

   DynamicObject context(NULL);
   _normalize(context, in, out);

   return rval;
}
