/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/util/DynamicObjectImpl.h"
#include "db/util/DynamicObject.h"

using namespace std;
using namespace db::util;

DynamicObjectImpl::DynamicObjectImpl()
{
   mType = String;
   mString = strdup("");
}

DynamicObjectImpl::~DynamicObjectImpl()
{
   DynamicObjectImpl::freeData();
}

void DynamicObjectImpl::freeData()
{
   // clean up data based on type
   switch(mType)
   {
      case String:
         delete [] mString;
         mString = NULL;
         break;
      case Map:
         if(mMap != NULL)
         {
            // clean up member names
            for(ObjectMap::iterator i = mMap->begin(); i != mMap->end(); i++)
            {
               delete [] i->first;
            }
            
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

DynamicObject& DynamicObjectImpl::operator[](const std::string& name)
{
   DynamicObject* rval = NULL;
   
   // change to map type if necessary
   setType(Map);
   
   ObjectMap::iterator i = mMap->find(name.c_str());
   if(i == mMap->end())
   {
      // create new map entry
      DynamicObject dyno;
      mMap->insert(std::make_pair(strdup(name.c_str()), dyno));
      rval = &(*mMap)[name.c_str()];
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

void DynamicObjectImpl::setType(DynamicObjectType type)
{
   if(getType() != type)
   {
      switch(type)
      {
         case String:
            getString();
            break;
         case Boolean:
            getBoolean();
            break;
         case Int32:
            getInt32();
            break;
         case UInt32:
            getUInt32();
            break;
         case Int64:
            getInt64();
            break;
         case UInt64:
            getUInt64();
            break;
         case Double:
            getDouble();
            break;
         case Map:
            if(mType != Map)
            {
               freeData();
               mType = Map;
               mMap = new ObjectMap();
            }
            break;
         case Array:
            // change to array type
            if(mType != Array)
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
   if(mType != String)
   {
      string str;
      toString(str);
      setString(str.c_str());
   }
   
   return mString;
}

bool DynamicObjectImpl::getBoolean()
{
   if(mType != Boolean)
   {
      // convert type as appropriate
      if(mType == String && mString != NULL)
      {
         *this = (strcmp(mString, "true") == 0);
      }
      else
      {
         switch(mType)
         {
            case Int32:
               *this = (mInt32 == 1);
               break;
            case UInt32:
               *this = (mUInt32 == 1);
               break;
            case Int64:
               *this = (mInt64 == 1);
               break;
            case UInt64:
               *this = (mUInt64 == 1);
               break;
            case Double:
               *this = (mDouble == 1);
               break;
            default:
               *this = false;
               break;
         }
      }
   }
   
   return mBoolean;
}

int DynamicObjectImpl::getInt32()
{
   if(mType != Int32)
   {
      // convert type as appropriate
      if(mType == String && mString != NULL)
      {
         *this = (int)strtol(mString, NULL, 10);
      }
      else if(mType == Boolean)
      {
         *this = mBoolean ? (int)1 : (int)0;
      }
      else if(mType == UInt32)
      {
         *this = (int)mUInt64;
      }
      else if(mType == Int64)
      {
         *this = (int)mInt64;
      }
      else if(mType == UInt64)
      {
         *this = (int)mUInt64;
      }
      else if(mType == Double)
      {
         *this = (int)mDouble;
      }
      else
      {
         *this = (int)0;
      }
   }
   
   return mInt32;
}

unsigned int DynamicObjectImpl::getUInt32()
{
   if(mType != UInt32)
   {
      // convert type as appropriate
      if(mType == String && mString != NULL)
      {
         *this = (unsigned int)strtoul(mString, NULL, 10);
      }
      else if(mType == Boolean)
      {
         *this = mBoolean ? (unsigned int)1 : (unsigned int)0;
      }
      else if(mType == Int32 && mInt32 > 0)
      {
         *this = (unsigned int)mInt32;
      }
      else if(mType == Int64 && mInt64 > 0)
      {
         *this = (unsigned int)mInt64;
      }
      else if(mType == UInt64)
      {
         *this = (unsigned int)mUInt64;
      }
      else if(mType == Double && mDouble > 0)
      {
         *this = (unsigned int)mDouble;
      }
      else
      {
         *this = (unsigned int)0;
      }
   }
   
   return mUInt32;
}

long long DynamicObjectImpl::getInt64()
{
   if(mType != Int64)
   {
      // convert type as appropriate
      if(mType == String && mString != NULL)
      {
         *this = (long long)strtoll(mString, NULL, 10);
      }
      else if(mType == Boolean)
      {
         *this = mBoolean ? (long long)1 : (long long)0;
      }
      else if(mType == Int32)
      {
         *this = (long long)mInt32;
      }
      else if(mType == UInt32)
      {
         *this = (long long)mUInt32;
      }
      else if(mType == UInt64)
      {
         *this = (long long)mUInt64;
      }
      else if(mType == Double)
      {
         *this = (long long)mDouble;
      }
      else
      {
         *this = (long long)0;
      }
   }
   
   return mInt64;
}

unsigned long long DynamicObjectImpl::getUInt64()
{
   if(mType != UInt64)
   {
      // convert type as appropriate
      if(mType == String && mString != NULL)
      {
         *this = (unsigned long long)strtoull(mString, NULL, 10);
      }
      else if(mType == Boolean)
      {
         *this = mBoolean ? (unsigned long long)1 : (unsigned long long)0;
      }
      else if(mType == Int32 && mInt32 > 0)
      {
         *this = (unsigned long long)mInt32;
      }
      else if(mType == UInt32)
      {
         *this = (unsigned long long)mUInt32;
      }
      else if(mType == Int64 && mInt64 > 0)
      {
         *this = (unsigned long long)mInt64;
      }
      else if(mType == Double && mDouble > 0)
      {
         *this = (unsigned long long)mDouble;
      }
      else
      {
         *this = (unsigned long long)0;
      }
   }
   
   return mUInt64;
}

double DynamicObjectImpl::getDouble()
{
   if(mType != Double)
   {
      // convert type as appropriate
      if(mType == String && mString != NULL)
      {
         *this = (double)strtod(mString, NULL);
      }
      else if(mType == Boolean)
      {
         *this = mBoolean ? (double)1 : (double)0;
      }
      else if(mType == Int32)
      {
         *this = (double)mInt32;
      }
      else if(mType == UInt32)
      {
         *this = (double)mUInt32;
      }
      else if(mType == Int64)
      {
         *this = (double)mInt64;
      }
      else if(mType == UInt64)
      {
         *this = (double)mUInt64;
      }
      else
      {
         *this = (double)0;
      }
   }
   
   return mDouble;
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
         delete [] i->first;
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

string& DynamicObjectImpl::toString(string& str) const
{
   if(mType != String)
   {
      // convert type as appropriate
      char temp[22];
      switch(mType)
      {
         case Boolean:
            sprintf(temp, "%s", (mBoolean ? "true" : "false"));
            break;
         case Int32:
            sprintf(temp, "%i", mInt32);
            break;
         case UInt32:
            sprintf(temp, "%u", mUInt32);
            break;
         case Int64:
            sprintf(temp, "%lli", mInt64);
            break;
         case UInt64:
            sprintf(temp, "%llu", mUInt64);
            break;
         case Double:
            sprintf(temp, "%e", mDouble);
            break;
         default: /* Map, Array, ... */
            temp[0] = 0;
            break;
      }
      str.assign(temp);
   }
   else
   {
      if(mString == NULL)
      {
         str.assign("");
      }
      else
      {
         str.assign(mString);
      }
   }
   
   return str;
}
