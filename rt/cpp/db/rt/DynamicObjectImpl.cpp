/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/rt/DynamicObjectImpl.h"

#include "db/rt/DynamicObject.h"

using namespace std;
using namespace db::rt;

DynamicObjectImpl::DynamicObjectImpl()
{
   mType = String;
   mString = NULL;
   mStringValue = NULL;
}

DynamicObjectImpl::~DynamicObjectImpl()
{
   DynamicObjectImpl::freeData();
   
   // free cached string value
   if(mStringValue != NULL)
   {
      free(mStringValue);
   }
}

void DynamicObjectImpl::freeMapKeys()
{
   // clean up member names
   for(ObjectMap::iterator i = mMap->begin(); i != mMap->end(); i++)
   {
      free((char*)i->first);
   }
}

void DynamicObjectImpl::freeData()
{
   // clean up data based on type
   switch(mType)
   {
      case String:
         {
            if(mString != NULL)
            {
               free(mString);
            }
            mString = NULL;
         }
         break;
      case Map:
         if(mMap != NULL)
         {
            freeMapKeys();
            delete mMap;
            mMap = NULL;
         }
         break;
      case Array:
         if(mArray != NULL)
         {
            delete mArray;
            mArray = NULL;
         }
         break;
      default:
         // nothing to cleanup
         break;
   }
}

void DynamicObjectImpl::setString(const char* value)
{
   freeData();
   mType = String;
   mString = strdup(value);
}

void DynamicObjectImpl::operator=(const char* value)
{
   setString(value);
}

void DynamicObjectImpl::operator=(bool value)
{
   freeData();
   mType = Boolean;
   mBoolean = value;
}

void DynamicObjectImpl::operator=(int value)
{
   freeData();
   mType = Int32;
   mInt32 = value;
}

void DynamicObjectImpl::operator=(unsigned int value)
{
   freeData();
   mType = UInt32;
   mUInt32 = value;
}

void DynamicObjectImpl::operator=(long long value)
{
   freeData();
   mType = Int64;
   mInt64 = value;
}

void DynamicObjectImpl::operator=(unsigned long long value)
{
   freeData();
   mType = UInt64;
   mUInt64 = value;
}

void DynamicObjectImpl::operator=(double value)
{
   freeData();
   mType = Double;
   mDouble = value;
}

DynamicObject& DynamicObjectImpl::operator[](const char* name)
{
   DynamicObject* rval = NULL;
   
   // change to map type if necessary
   setType(Map);
   
   ObjectMap::iterator i = mMap->find(name);
   if(i == mMap->end())
   {
      // create new map entry
      DynamicObject dyno;
      mMap->insert(std::make_pair(strdup(name), dyno));
      rval = &(*mMap)[name];
   }
   else
   {
      // get existing map entry
      rval = &i->second;
   }
   
   return *rval;
}

DynamicObject& DynamicObjectImpl::operator[](int index)
{
   // change to array type if necessary
   setType(Array);
   
   if(index < 0)
   {
      index = mArray->size() + index;
   }
   
   // fill the object array as necessary
   if(index >= (int)mArray->size())
   {
      int i = index - (int)mArray->size() + 1;
      for(; i > 0; i--)
      {
         DynamicObject dyno;
         mArray->push_back(dyno);
      }
   }
   
   return (*mArray)[index];
}

DynamicObject& DynamicObjectImpl::append()
{
   return (*this)[length()];
}

void DynamicObjectImpl::setType(DynamicObjectType type)
{
   if(mType != type)
   {
      switch(type)
      {
         case String:
            *this = getString();
            break;
         case Boolean:
            *this = getBoolean();
            break;
         case Int32:
            *this = getInt32();
            break;
         case UInt32:
            *this = getUInt32();
            break;
         case Int64:
            *this = getInt64();
            break;
         case UInt64:
            *this = getUInt64();
            break;
         case Double:
            *this = getDouble();
            break;
         case Map:
            {
               freeData();
               mType = Map;
               mMap = new ObjectMap();
            }
            break;
         case Array:
            {
               freeData();
               mType = Array;
               mArray = new ObjectArray();
            }
            break;
      }
   }
}

DynamicObjectType DynamicObjectImpl::getType()
{
   return mType;
}

const char* DynamicObjectImpl::getString()
{
   const char* rval;
   
   if(mType == String)
   {
      if(mString != NULL)
      {
         // use existing string
         rval = mString;
      }
      else
      {
         // only duplicate blank string upon request
         rval = mString = strdup("");
      }
   }
   else
   {
      // convert type as appropriate
      switch(mType)
      {
         case Boolean:
            mStringValue = (char*)realloc(mStringValue, 6);
            snprintf(mStringValue, 6, "%s", (mBoolean ? "true" : "false"));
            break;
         case Int32:
            mStringValue = (char*)realloc(mStringValue, 12);
            snprintf(mStringValue, 12, "%i", mInt32);
            break;
         case UInt32:
            mStringValue = (char*)realloc(mStringValue, 11);
            snprintf(mStringValue, 11, "%u", mUInt32);
            break;
         case Int64:
            mStringValue = (char*)realloc(mStringValue, 22);
            snprintf(mStringValue, 22, "%lli", mInt64);
            break;
         case UInt64:
            mStringValue = (char*)realloc(mStringValue, 21);
            snprintf(mStringValue, 21, "%llu", mUInt64);
            break;
         case Double:
            // use default precision of 6
            // X.000000e+00 = 11 places to right of decimal
            mStringValue = (char*)realloc(mStringValue, 50);
            snprintf(mStringValue, 50, "%e", mDouble);
            break;
         default: /* Map, Array, ... */
            {
               if(mStringValue == NULL)
               {
                  // duplicate blank string
                  mStringValue = strdup("");
               }
               else
               {
                  // set null-terminator to first character
                  mStringValue[0] = 0; 
               }
            }
            break;
      }
      
      // return generated value
      rval = mStringValue;
   }
   
   return rval;
}

bool DynamicObjectImpl::getBoolean()
{
   bool rval;
   
   switch(mType)
   {
      case Boolean:
         rval = mBoolean;
         break;
      case String:
         rval = (mString == NULL) ? false : (strcmp(mString, "true") == 0);
         break;
      case Int32:
         rval = (mInt32 == 1);
         break;
      case UInt32:
         rval = (mUInt32 == 1);
         break;
      case Int64:
         rval = (mInt64 == 1);
         break;
      case UInt64:
         rval = (mUInt64 == 1);
         break;
      case Double:
         rval = (mDouble == 1);
         break;
      default:
         rval = false;
         break;
   }
   
   return rval;
}

int DynamicObjectImpl::getInt32()
{
   int rval;
   
   switch(mType)
   {
      case Int32:
         rval = mInt32;
         break;
      case String:
         rval = (mString == NULL) ? 0 : strtol(mString, NULL, 10);
         break;
      case Boolean:
         rval = mBoolean ? 1 : 0;
         break;
      case UInt32:
         rval = (int)mUInt32;
         break;
      case Int64:
         rval = (int)mInt64;
         break;
      case UInt64:
         rval = (int)mUInt64;
         break;
      case Double:
         rval = (int)mDouble;
         break;
      default:
         rval = 0;
         break;
   }
   
   return rval;
}

unsigned int DynamicObjectImpl::getUInt32()
{
   unsigned int rval;
   
   switch(mType)
   {
      case UInt32:
         rval = mUInt32;
         break;
      case String:
         rval = (mString == NULL) ? 0 : strtoul(mString, NULL, 10);
         break;
      case Boolean:
         rval = mBoolean ? 1 : 0;
         break;
      case Int32:
         rval = (mInt32 < 0) ? 0 : mInt32;
         break;
      case Int64:
         rval = (mInt64 < 0) ? 0 : (unsigned int)mInt64;
         break;
      case UInt64:
         rval = (unsigned int)mUInt64;
         break;
      case Double:
         rval = (unsigned int)mDouble;
         break;
      default:
         rval = 0;
         break;
   }
   
   return rval;
}

long long DynamicObjectImpl::getInt64()
{
   long long rval;
   
   switch(mType)
   {
      case Int64:
         rval = mInt64;
         break;
      case String:
         rval = (mString == NULL) ? 0 : strtoll(mString, NULL, 10);
         break;
      case Boolean:
         rval = mBoolean ? 1 : 0;
         break;
      case Int32:
         rval = mInt32;
         break;
      case UInt32:
         rval = mUInt32;
         break;
      case UInt64:
         rval = (long long)mUInt64;
         break;
      case Double:
         rval = (long long)mDouble;
         break;
      default:
         rval = 0;
         break;
   }
   
   return rval;
}

unsigned long long DynamicObjectImpl::getUInt64()
{
   unsigned long long rval;
   
   switch(mType)
   {
      case UInt64:
         rval = mUInt64;
         break;
      case String:
         rval = (mString == NULL) ? 0 : strtoull(mString, NULL, 10);
         break;
      case Boolean:
         rval = mBoolean ? 1 : 0;
         break;
      case Int32:
         rval = (mInt32 < 0) ? 0 : mInt32;
         break;
      case UInt32:
         rval = mUInt32;
         break;
      case Int64:
         rval = (mInt64 < 0) ? 0 : mInt64;
         break;
      case Double:
         rval = mDouble;
         break;
      default:
         rval = 0;
         break;
   }
   
   return rval;
}

double DynamicObjectImpl::getDouble()
{
   double rval;
   
   switch(mType)
   {
      case Double:
         rval = mDouble;
         break;
      case String:
         rval = (mString == NULL) ? 0 : strtod(mString, NULL);
         break;
      case Boolean:
         rval = mBoolean ? 1 : 0;
         break;
      case Int32:
         rval = mInt32;
         break;
      case UInt32:
         rval = mUInt32;
         break;
      case Int64:
         rval = mInt64;
         break;
      case UInt64:
         rval = mUInt64;
         break;
      default:
         rval = 0;
         break;
   }
   
   return rval;
}

bool DynamicObjectImpl::hasMember(const char* name)
{
   bool rval = false;
   
   if(mType == Map)
   {
      ObjectMap::iterator i = mMap->find(name);
      rval = (i != mMap->end());
   }
   
   return rval;
}

DynamicObject DynamicObjectImpl::removeMember(const char* name)
{
   DynamicObject rval(NULL);
   
   if(mType == Map)
   {
      ObjectMap::iterator i = mMap->find(name);
      if(i != mMap->end())
      {
         // clean up key and remove map entry
         free((char*)i->first);
         rval = i->second;
         mMap->erase(i);
      }
   }
   
   return rval;
}

void DynamicObjectImpl::clear()
{
   switch(mType)
   {
      case String:
         *this = "";
         break;
      case Boolean:
         *this = false;
         break;
      case Int32:
         *this = (int)0;
         break;
      case UInt32:
         *this = (unsigned int)0;
         break;
      case Int64:
         *this = (long long)0;
         break;
      case UInt64:
         *this = (unsigned long long)0;
         break;
      case Double:
         *this = (double)0.0;
         break;
      case Map:
         freeMapKeys();
         mMap->clear();
         break;
      case Array:
         mArray->clear();
         break;
   }
}

int DynamicObjectImpl::length()
{
   int rval = 0;
   
   switch(mType)
   {
      case String:
         if(mString != NULL)
         {
            rval = strlen(getString());
         }
         break;
      case Boolean:
         rval = 1;
         break;
      case Int32:
      case UInt32:
         rval = sizeof(unsigned int);
         break;
      case Int64:
      case UInt64:
         rval = sizeof(unsigned long long);
         break;
      case Double:
         rval = sizeof(double);
         break;
      case Map:
         rval = mMap->size();
         break;
      case Array:
         rval = mArray->size();
         break;
   }
   
   return rval;
}
