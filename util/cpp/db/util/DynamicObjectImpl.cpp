/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/util/DynamicObjectImpl.h"
#include "db/util/DynamicObject.h"

using namespace db::util;

DynamicObjectImpl::DynamicObjectImpl()
{
   mType = String;
   mString = NULL;
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
         if(mString != NULL)
         {
            delete [] mString;
            mString = NULL;
         }
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

void DynamicObjectImpl::operator=(const std::string& value)
{
   setString(value.c_str());
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

DynamicObject& DynamicObjectImpl::operator[](const std::string& name)
{
   DynamicObject* rval = NULL;
   
   // change to map type if necessary
   if(mType != Map)
   {
      freeData();
      mType = Map;
      mMap = new ObjectMap();
   }
   
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

DynamicObject& DynamicObjectImpl::operator[](unsigned int index)
{
   // change to array type if necessary
   if(mType != Array)
   {
      freeData();
      mType = Array;
      mArray = new ObjectArray();
   }
   
   // fill the object array as necessary
   if(index >= mArray->size())
   {
      for(unsigned int i = 0; i <= index; i++)
      {
         DynamicObject dyno;
         mArray->push_back(dyno);
      }
   }
   
   return (*mArray)[index];
}

DynamicObjectImpl::Type DynamicObjectImpl::getType()
{
   return mType;
}

const char* DynamicObjectImpl::getString()
{
   if(mType != String)
   {
      // convert type as appropriate
      char temp[22];
      switch(mType)
      {
         case Boolean:
            setString(mBoolean ? "true" : "false");
            break;
         case Int32:
            sprintf(temp, "%i", mInt32);
            setString(temp);
            break;
         case UInt32:
            sprintf(temp, "%u", mUInt32);
            setString(temp);
            break;
         case Int64:
            sprintf(temp, "%lli", mInt64);
            setString(temp);
            break;
         case UInt64:
            sprintf(temp, "%llu", mUInt64);
            setString(temp);
            break;
         default:
            setString("");
            break;
      }
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
      else if(mType == Int64)
      {
         *this = (int)mInt64;
      }
      else
      {
         *this = (int)0;
      }
   }
   
   return mInt32;
}

int DynamicObjectImpl::getUInt32()
{
   if(mType != UInt32)
   {
      // convert type as appropriate
      if(mType == String && mString != NULL)
      {
         *this = (unsigned int)strtoul(mString, NULL, 10);
      }
      else if(mType == UInt64)
      {
         *this = (unsigned int)mUInt64;
      }
      else
      {
         *this = (unsigned int)0;
      }
   }
   
   return mUInt32;
}

int DynamicObjectImpl::getInt64()
{
   if(mType != Int64)
   {
      // convert type as appropriate
      if(mType == String && mString != NULL)
      {
         *this = (long long)strtoll(mString, NULL, 10);
      }
      else
      {
         *this = (long long)0;
      }
   }
   
   return mInt64;
}

int DynamicObjectImpl::getUInt64()
{
   if(mType != UInt64)
   {
      // convert type as appropriate
      if(mType == String && mString != NULL)
      {
         *this = (unsigned long long)strtoull(mString, NULL, 10);
      }
      else
      {
         *this = (unsigned long long)0;
      }
   }
   
   return mUInt64;
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

unsigned int DynamicObjectImpl::length()
{
   unsigned int rval = 0;
   
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
         rval = 4;
         break;
      case Int64:
      case UInt64:
         rval = 8;
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
