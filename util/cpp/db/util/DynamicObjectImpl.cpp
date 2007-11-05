/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/util/DynamicObjectImpl.h"
#include "db/util/DynamicObject.h"

using namespace db::util;

DynamicObjectImpl::DynamicObjectImpl()
{
   mType = String;
   mString = strdup("");
   mName = strdup("");
}

DynamicObjectImpl::~DynamicObjectImpl()
{
   DynamicObjectImpl::freeData();
   
   // clean up name
   delete [] mName;
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
      delete [] dyno->mName;
      dyno->mName = strdup(name.c_str());
      mMap->insert(std::make_pair(strdup(name.c_str()), dyno));
      rval = &(*mMap)[name.c_str()];
   }
   else
   {
      // get existing map entry
      rval = &i->second;
      
      // update name as appropriate
      if(strcmp((*rval)->getName(), name.c_str()) != 0)
      {
         delete [] (*rval)->mName;
         (*rval)->mName = strdup(name.c_str());
      }
   }
   
   return *rval;
}

DynamicObject& DynamicObjectImpl::operator[](int index)
{
   // change to array type if necessary
   if(mType != Array)
   {
      freeData();
      mType = Array;
      mArray = new ObjectArray();
   }
   
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

DynamicObjectType DynamicObjectImpl::getType()
{
   return mType;
}

const char* DynamicObjectImpl::getName()
{
   return mName;
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
         case Double:
            sprintf(temp, "%e", mDouble);
            setString(temp);
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
      else if(mType == Int64)
      {
         *this = (int)mInt64;
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

long long DynamicObjectImpl::getInt64()
{
   if(mType != Int64)
   {
      // convert type as appropriate
      if(mType == String && mString != NULL)
      {
         *this = (long long)strtoll(mString, NULL, 10);
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
