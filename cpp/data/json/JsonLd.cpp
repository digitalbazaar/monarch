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
   DynamicObject& context, const char* str, char** tmp)
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
            if(*tmp == NULL || sizeof(*tmp) < len)
            {
               *tmp = (char*)realloc(*tmp, len);
            }
            snprintf(*tmp, len, "%s%s", uri->getString(), ptr + 1);
            rval = *tmp;
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
                  next, out[_normalizeString(context, i->getName(), &tmp)]);
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
         out = _normalizeString(context, in->getString(), &tmp);
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

/**
 * Denormalizes a string using the given context.
 *
 * @param context the context.
 * @param str the string to denormalize.
 * @param tmp to store the denormalized string, may be realloc'd.
 *
 * @return a pointer to the denormalized string.
 */
static const char* _denormalizeString(
   DynamicObject& context, const char* str, char* tmp)
{
   const char* rval = str;

   // check the context for a prefix that could shorten the string
   DynamicObjectIterator i = context.getIterator();
   while(rval == str && i->hasNext())
   {
      const char* uri = i->next()->getString();
      const char* ptr = strstr(str, uri);
      if(ptr != NULL && ptr == str)
      {
         size_t ulen = strlen(uri);
         size_t slen = strlen(str);
         if(slen > ulen)
         {
            // add 2 to make room for null-terminator and colon
            size_t total = strlen(i->getName()) + (slen - ulen) + 2;
            if(tmp == NULL || total > sizeof(tmp))
            {
               tmp = (char*)realloc(tmp, total);
            }
            snprintf(tmp, total, "%s:%s", i->getName(), ptr + ulen);
            rval = tmp;
         }
      }
   }

   return rval;
}

/**
 * Recursively denormalizes the given input object.
 *
 * @param context the context to use.
 * @param in the input object.
 * @param out the denormalized output object.
 */
static void _denormalize(
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

      if(inType == Map)
      {
         // denormalize each non-context property in the map
         char* tmp = NULL;
         DynamicObjectIterator i = in.getIterator();
         while(i->hasNext())
         {
            DynamicObject& next = i->next();
            if(strcmp(i->getName(), "#") != 0)
            {
               // denormalize key, denormalize object
               _denormalize(
                  context,
                  next, out[_denormalizeString(context, i->getName(), tmp)]);
            }
         }
         if(tmp != NULL)
         {
            free(tmp);
         }
      }
      else if(inType == Array)
      {
         // denormalize each object in the array
         DynamicObjectIterator i = in.getIterator();
         while(i->hasNext())
         {
            DynamicObject& next = i->next();
            _denormalize(context, next, out->append());
         }
      }
      // only strings need denormalization, numbers & booleans don't
      else if(inType == String)
      {
         // denormalize string
         char* tmp = NULL;
         out = _denormalizeString(context, in->getString(), tmp);
         if(tmp != NULL)
         {
            free(tmp);
         }
      }
   }
}

bool JsonLd::denormalize(
   DynamicObject& context, DynamicObject& in, DynamicObject& out)
{
   bool rval = true;

   out["#"] = context.clone();
   _denormalize(context, in, out);

   return rval;
}

bool JsonLd::changeContext(
   DynamicObject& context, DynamicObject& in, DynamicObject& out)
{
   // normalize "in" to remove its context then denormalize out
   DynamicObject tmp;
   return normalize(in, tmp) && denormalize(context, tmp, out);
}
