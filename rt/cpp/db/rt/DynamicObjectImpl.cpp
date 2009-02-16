/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#define __STDC_LIMIT_MACROS

#include "db/rt/DynamicObjectImpl.h"

#include "db/rt/DynamicObject.h"

#include <cstdlib>
#include <cstdio>
#include <string>
#include <algorithm>

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

void DynamicObjectImpl::operator=(const DynamicObjectImpl& value)
{
   switch(value.mType)
   {
      case String:
         *this = value.mString;
         break;
      case Boolean:
         *this = value.mBoolean;
         break;
      case Int32:
         *this = value.mInt32;
         break;
      case UInt32:
         *this = value.mUInt32;
         break;
      case Int64:
         *this = value.mInt64;
         break;
      case UInt64:
         *this = value.mUInt64;
         break;
      case Double:
         *this = value.mDouble;
         break;
      case Map:
      {
         setType(Map);
         clear();
         ObjectMap::iterator i = value.mMap->begin();
         for(; i != value.mMap->end(); i++)
         {
            // create new map entry
            mMap->insert(std::make_pair(strdup(i->first), i->second));
         }
         break;
      }
      case Array:
      {
         setType(Array);
         clear();
         ObjectArray::iterator i = value.mArray->begin();
         for(; i != value.mArray->end(); i++)
         {
            // create new array
            mArray->push_back(*i);
         }
         break;
      }
   }
}

void DynamicObjectImpl::operator=(const char* value)
{
   freeData();
   mType = String;
   mString = strdup(value);
}

void DynamicObjectImpl::operator=(bool value)
{
   freeData();
   mType = Boolean;
   mBoolean = value;
}

void DynamicObjectImpl::operator=(int32_t value)
{
   freeData();
   mType = Int32;
   mInt32 = value;
}

void DynamicObjectImpl::operator=(uint32_t value)
{
   freeData();
   mType = UInt32;
   mUInt32 = value;
}

void DynamicObjectImpl::operator=(int64_t value)
{
   freeData();
   mType = Int64;
   mInt64 = value;
}

void DynamicObjectImpl::operator=(uint64_t value)
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
   if(mType != Map)
   {
      setType(Map);
   }
   
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
   if(mType != Array)
   {
      setType(Array);
   }
   
   int size = mArray->size();
   
   // fill the object array as necessary
   if(index >= size)
   {
      int i = index - size + 1;
      for(; i > 0; i--)
      {
         DynamicObject dyno;
         mArray->push_back(dyno);
      }
   }
   // assume abs(index) >= size
   else if(index < 0)
   {
      index = size + index;
   }
   
   return (*mArray)[index];
}

bool DynamicObjectImpl::operator==(const DynamicObjectImpl& rhs) const
{
   bool rval;
   
   if(mType == rhs.mType)
   {
      switch(mType)
      {
         case String:
            rval = (strcmp(getString(), rhs.getString()) == 0);
            break;
         case Boolean:
            rval = (mBoolean == rhs.mBoolean);
            break;
         case Int32:
            rval = (mInt32 == rhs.mInt32);
            break;
         case UInt32:
            rval = (mUInt32 == rhs.mUInt32);
            break;
         case Int64:
            rval = (mInt64 == rhs.mInt64);
            break;
         case UInt64:
            rval = (mUInt64 == rhs.mUInt64);
            break;
         case Double:
            rval = (mDouble == rhs.mDouble);
            break;
         case Map:
            // ensure maps are the same length
            rval = (length() == rhs.length());
            if(rval)
            {
               // compare map keys first, then map values if keys are equal
               ObjectMap::iterator li = mMap->begin();
               ObjectMap::iterator ri = rhs.mMap->begin();
               for(; rval && li != mMap->end(); li++, ri++)
               {
                  if(strcmp(li->first, ri->first) == 0)
                  {
                     // compare key values
                     rval = (li->second == ri->second);
                  }
                  else
                  {
                     // keys not equal
                     rval = false;
                  }
               }            
            }
            break;
         case Array:
            rval = (*mArray == *(rhs.mArray));
            break;
      }
   }
   else
   {
      // compare based on string values
      switch(mType)
      {
         case String:
         case Boolean:
         case Int32:
         case Int64:
         case UInt32:
         case UInt64:
         case Double:
            switch(rhs.mType)
            {
               case String:
               case Boolean:
               case Int32:
               case UInt32:
               case Int64:
               case UInt64:
               case Double:
                  rval = (strcmp(getString(), rhs.getString()) == 0);
                  break;
               default:
                  rval = false;
                  break;
            }
            break;
         default:
            rval = false;
            break;
      }
   }
   
   return rval;
}

bool DynamicObjectImpl::operator<(const DynamicObjectImpl& rhs) const
{
   bool rval;
   
   if(mType == rhs.mType)
   {
      switch(mType)
      {
         case String:
            rval = (strcmp(getString(), rhs.getString()) < 0);
            break;
         case Boolean:
            // false is "less" than true
            rval = (!mBoolean && rhs.mBoolean);
            break;
         case Int32:
            rval = (mInt32 < rhs.mInt32);
            break;
         case UInt32:
            rval = (mUInt32 < rhs.mUInt32);
            break;
         case Int64:
            rval = (mInt64 < rhs.mInt64);
            break;
         case UInt64:
            rval = (mUInt64 < rhs.mUInt64);
            break;
         case Double:
            rval = (mDouble < rhs.mDouble);
            break;
         case Map:
            // a smaller map is "less"
            if(length() < rhs.length())
            {
               rval = true;
            }
            else if(length() > rhs.length())
            {
               rval = false;
            }
            else
            {
               // compare map keys first, then map values if keys are equal
               rval = false;
               ObjectMap::iterator li = mMap->begin();
               ObjectMap::iterator ri = rhs.mMap->begin();
               for(; !rval && li != mMap->end(); li++, ri++)
               {
                  int ret = strcmp(li->first, ri->first);
                  if(ret == 0)
                  {
                     // compare key values
                     if(li->second < ri->second)
                     {
                        rval = true;
                     }
                     else if(li->second != ri->second)
                     {
                        rval = false;
                        break;
                     }
                  }
                  else
                  {
                     // map key is less or greater
                     rval = (ret < 0);
                     break;
                  }
               }
            }
            break;
         case Array:
            rval = (*mArray == *(rhs.mArray));
            break;
      }
   }
   else
   {
      // compare based on string values
      switch(mType)
      {
         case String:
         case Boolean:
         case Int32:
         case Int64:
         case UInt32:
         case UInt64:
         case Double:
            switch(rhs.mType)
            {
               case String:
               case Boolean:
               case Int32:
               case UInt32:
               case Int64:
               case UInt64:
               case Double:
                  rval = (strcmp(getString(), rhs.getString()) < 0);
                  break;
               default:
                  // maps/arrays "greater/equal" to other types
                  rval = false;
                  break;
            }
            break;
         case Map:
            // map is "less" than array, nothing else
            rval = (rhs.mType == Array);
            break;
         case Array:
            // array is "greatest" type
            rval = false;
            break;
      }
   }
   
   return rval;
}

DynamicObject& DynamicObjectImpl::append()
{
   if(mType != Array)
   {
      setType(Array);
   }
   
   DynamicObject d;
   mArray->push_back(d);
   return mArray->back();
}

DynamicObject& DynamicObjectImpl::append(DynamicObject& dyno)
{
   if(mType != Array)
   {
      setType(Array);
   }
   
   mArray->push_back(dyno);
   return mArray->back();
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
            break;
         }
         case Array:
         {
            freeData();
            mType = Array;
            mArray = new ObjectArray();
            break;
         }
      }
   }
}

DynamicObjectType DynamicObjectImpl::getType() const
{
   return mType;
}

void DynamicObjectImpl::format(const char* format, ...)
{
   va_list varargs;
   va_start(varargs, format);
   setFormattedString(format, varargs);
   va_end(varargs);
}

const char* DynamicObjectImpl::getString() const
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
         // return blank string
         rval = "";
      }
   }
   else if(mType == Map || mType == Array)
   {
      // return blank string
      rval = "";
   }
   else
   {
      char* str = (char*)(mStringValue);
      
      // convert type as appropriate
      switch(mType)
      {
         case Boolean:
            str = (char*)realloc(str, 6);
            snprintf(str, 6, "%s", (mBoolean ? "true" : "false"));
            break;
         case Int32:
            str = (char*)realloc(str, 12);
            snprintf(str, 12, "%i", mInt32);
            break;
         case UInt32:
            str = (char*)realloc(str, 11);
            snprintf(str, 11, "%u", mUInt32);
            break;
         case Int64:
            str = (char*)realloc(str, 22);
            snprintf(str, 22, "%lli", mInt64);
            break;
         case UInt64:
            str = (char*)realloc(str, 21);
            snprintf(str, 21, "%llu", mUInt64);
            break;
         case Double:
            // use default precision of 6
            // X.000000e+00 = 11 places to right of decimal
            str = (char*)realloc(str, 50);
            snprintf(str, 50, "%e", mDouble);
            break;
         default: /* String, Map, Array, ... already handled*/
            break;
      }
      
      // set generated value
      ((DynamicObjectImpl*)this)->mStringValue = str;
      
      // return generated value
      rval = mStringValue;
   }
   
   return rval;
}

bool DynamicObjectImpl::getBoolean() const
{
   bool rval;
   
   switch(mType)
   {
      case Boolean:
         rval = mBoolean;
         break;
      case String:
         rval = (strcmp(getString(), "true") == 0);
         break;
      case Int32:
         rval = !(mInt32 == 0);
         break;
      case UInt32:
         rval = !(mUInt32 == 0);
         break;
      case Int64:
         rval = !(mInt64 == 0);
         break;
      case UInt64:
         rval = !(mUInt64 == 0);
         break;
      case Double:
         rval = !(mDouble == 0.0);
         break;
      default:
         rval = false;
         break;
   }
   
   return rval;
}

int32_t DynamicObjectImpl::getInt32() const
{
   int32_t rval;
   
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
         rval = (int32_t)((mUInt32 > INT32_MAX) ? INT32_MAX : mUInt32);
         break;
      case Int64:
         rval = (int32_t)((mInt64 > INT32_MAX) ? INT32_MAX :
            ((mInt64 < INT32_MIN) ? INT32_MIN : mInt64));
         break;
      case UInt64:
         rval = (int32_t)((mUInt64 > INT32_MAX) ? INT32_MAX : mUInt64);
         break;
      case Double:
         rval = (int32_t)mDouble;
         break;
      default:
         rval = 0;
         break;
   }
   
   return rval;
}

uint32_t DynamicObjectImpl::getUInt32() const
{
   uint32_t rval;
   
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
         rval = (uint32_t)((mInt32 < 0) ? 0 : mInt32);
         break;
      case Int64:
         rval = (uint32_t)((mInt64 < 0) ? 0 :
            ((mInt64 > UINT32_MAX) ? UINT32_MAX : mInt64));
         break;
      case UInt64:
         rval = (uint32_t)((mUInt64 > UINT32_MAX) ? UINT32_MAX : mUInt64);
         break;
      case Double:
         rval = (uint32_t)mDouble;
         break;
      default:
         rval = 0;
         break;
   }
   
   return rval;
}

int64_t DynamicObjectImpl::getInt64() const
{
   int64_t rval;
   
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
         rval = (int64_t)mInt32;
         break;
      case UInt32:
         rval = (int64_t)mUInt32;
         break;
      case UInt64:
         rval = (int64_t)((mUInt64 > INT64_MAX) ? INT64_MAX : mUInt64);
         break;
      case Double:
         rval = (int64_t)mDouble;
         break;
      default:
         rval = 0;
         break;
   }
   
   return rval;
}

uint64_t DynamicObjectImpl::getUInt64() const
{
   uint64_t rval;
   
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
         rval = (uint64_t)((mInt32 < 0) ? 0 : mInt32);
         break;
      case UInt32:
         rval = (uint64_t)mUInt32;
         break;
      case Int64:
         rval = (uint64_t)((mInt64 < 0) ? 0 : mInt64);
         break;
      case Double:
         rval = (uint64_t)mDouble;
         break;
      default:
         rval = 0;
         break;
   }
   
   return rval;
}

double DynamicObjectImpl::getDouble() const
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

bool DynamicObjectImpl::hasMember(const char* name) const
{
   bool rval = false;
   
   if(mType == Map)
   {
      rval = (mMap->count(name) != 0);
   }
   
   return rval;
}

int DynamicObjectImpl::indexOf(DynamicObject& obj) const
{
   int rval = -1;
   
   // type must be array to get an index
   if(mType == Array)
   {
      ObjectArray::iterator i = find(mArray->begin(), mArray->end(), obj);
      if(i != mArray->end())
      {
         rval = (i - mArray->begin());
      }
   }
   
   return rval;
}

void DynamicObjectImpl::removeMember(const char* name)
{
   if(mType == Map)
   {
      ObjectMap::iterator i = mMap->find(name);
      if(i != mMap->end())
      {
         removeMember(i);
      }
   }
}

void DynamicObjectImpl::clear()
{
   switch(mType)
   {
      case String:
         *this = "";
         break;
      case Boolean:
         mBoolean = false;
         break;
      case Int32:
         mInt32 = 0;
         break;
      case UInt32:
         mUInt32 = 0;
         break;
      case Int64:
         mInt64 = 0;
         break;
      case UInt64:
         mUInt64 = 0;
         break;
      case Double:
         mDouble = 0.0;
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

int DynamicObjectImpl::length() const
{
   int rval;
   
   switch(mType)
   {
      case String:
         rval = strlen(getString());
         break;
      case Boolean:
         rval = 1;
         break;
      case Int32:
      case UInt32:
         rval = sizeof(uint32_t);
         break;
      case Int64:
      case UInt64:
         rval = sizeof(uint64_t);
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

void DynamicObjectImpl::reverse()
{
   switch(mType)
   {
      case String:
         if(mString != NULL)
         {
            string s = mString;
            std::reverse(s.begin(), s.end());
            *this = s.c_str();
         }
         break;
      case Array:
         std::reverse(mArray->begin(), mArray->end());
         break;
      default:
         break;
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
         if(mString != NULL)
         {
            free(mString);
            mString = NULL;
         }
         break;
      case Map:
         if(mMap != NULL)
         {
            freeMapKeys();
            mMap->clear();
            delete mMap;
            mMap = NULL;
         }
         break;
      case Array:
         if(mArray != NULL)
         {
            mArray->clear();
            delete mArray;
            mArray = NULL;
         }
         break;
      default:
         // nothing to cleanup
         break;
   }
}

void DynamicObjectImpl::removeMember(ObjectMap::iterator iterator)
{
   // clean up key and remove map entry
   free((char*)iterator->first);
   mMap->erase(iterator);
}

void DynamicObjectImpl::setFormattedString(const char* format, va_list varargs)
{
   // Note: this code is adapted from the glibc sprintf documentation
   
   // estimate 256 bytes to start with
   int n, size = 256;
   char *p;
   char *np;
   
   bool mallocFailed = ((p = (char*)malloc(size)) == NULL);
   bool success = false;
   while(!success && !mallocFailed)
   {
      // try to print in the allocated space
      n = vsnprintf(p, size, format, varargs);
      
      // if that worked, return the string
      if(n > -1 && n < size)
      {
         success = true;
      }
      else
      {
         // try again with more space
         if(n > -1)
         {
            // glibc 2.1 says (n + 1) is exactly what is needed
            size = n + 1;
         }
         else
         {
            // glibc 2.0 doesn't know the exact size, so guess
            size *= 2;
         }
         
         if((np = (char*)realloc(p, size)) == NULL)
         {
            // bad malloc
            free(p);
            mallocFailed = true;
         }
         else
         {
            p = np;
         }
      }
   }
   
   if(success)
   {
      operator=(p);
      free(p);
   }
}
