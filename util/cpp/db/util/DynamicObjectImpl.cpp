/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/util/DynamicObjectImpl.h"
#include "db/util/DynamicObject.h"

using namespace db::util;

DynamicObjectImpl::MemberValue::MemberValue()
{
   type = String;
   str = NULL;
}

void DynamicObjectImpl::MemberValue::freeData()
{
   // clean up data based on type
   switch(type)
   {
      case Object:
         delete obj;
         obj = NULL;
         break;
      case Array:
         // free every element in array
         for(std::vector<MemberValue>::iterator i = array->begin();
             i != array->end(); i++)
         {
            i->freeData();
         }
         
         // free array
         delete array;
         array = NULL;
         break;
      default:
         if(str != NULL)
         {
            delete [] str;
            str = NULL;
         }
         break;
   }
}

void DynamicObjectImpl::MemberValue::operator=(const char* rhs)
{
   freeData();
   type = String;
   str = strdup(rhs);
}

void DynamicObjectImpl::MemberValue::operator=(bool rhs)
{
   freeData();
   type = Boolean;
   str = new char[6];
   strcpy(str, rhs ? "true" : "false");
}

void DynamicObjectImpl::MemberValue::operator=(int rhs)
{
   freeData();
   type = Int32;
   str = new char[22];
   sprintf(str, "%i", rhs);
}

void DynamicObjectImpl::MemberValue::operator=(unsigned int rhs)
{
   freeData();
   type = UInt32;
   str = new char[22];
   sprintf(str, "%u", rhs);
}

void DynamicObjectImpl::MemberValue::operator=(long long rhs)
{
   freeData();
   type = Int64;
   str = new char[22];
   sprintf(str, "%lli", rhs);
}

void DynamicObjectImpl::MemberValue::operator=(unsigned long long rhs)
{
   freeData();
   type = UInt64;
   str = new char[22];
   sprintf(str, "%llu", rhs);
}

void DynamicObjectImpl::MemberValue::operator=(DynamicObject rhs)
{
   freeData();
   type = Object;
   obj = new DynamicObject(rhs);
}

DynamicObjectImpl::MemberValue& DynamicObjectImpl::MemberValue::operator[](
   const std::string& name)
{
   // create object if necessary
   if(type != Object)
   {
      freeData();
      type = Object;
      obj = new DynamicObject();
   }
   
   return (*obj)[name.c_str()];
}

DynamicObjectImpl::MemberValue& DynamicObjectImpl::MemberValue::operator[](
   unsigned int index)
{
   // create array if necessary
   if(type != Array)
   {
      freeData();
      type = Array;
      array = new std::vector<MemberValue>();
   }
   
   // create MemberValues for every value up to the given index
   if(index >= array->size())
   {
      for(unsigned int i = 0; i <= index; i++)
      {
         MemberValue mvnew;
         array->push_back(mvnew);
      }
   }
   
   return (*array)[index];
}

DynamicObjectImpl::DynamicObjectImpl()
{
}

DynamicObjectImpl::~DynamicObjectImpl()
{
   // clean up members
   for(MemberMap::iterator i = mMembers.begin(); i != mMembers.end(); i++)
   {
      // clean up member name
      delete [] i->first;
      
      // clean up member value data
      i->second.freeData();
   }
}

DynamicObjectImpl::MemberValue& DynamicObjectImpl::operator[](const char* name)
{
   MemberValue* rval = NULL;
   
   MemberMap::iterator i = mMembers.find(name);
   if(i != mMembers.end())
   {
      rval = &i->second;
   }
   else
   {
      // create new member value
      MemberValue mvnew;
      mMembers[strdup(name)] = mvnew;
      rval = &mMembers[name];
   }
   
   return *rval;
}

void DynamicObjectImpl::setMember(const char* name, const char* value)
{
   (*this)[name] = value;
}

void DynamicObjectImpl::setMember(const char* name, bool value)
{
   (*this)[name] = value;
}

void DynamicObjectImpl::setMember(const char* name, int value)
{
   (*this)[name] = value;
}

void DynamicObjectImpl::setMember(const char* name, unsigned int value)
{
   (*this)[name] = value;
}

void DynamicObjectImpl::setMember(const char* name, long long value)
{
   (*this)[name] = value;
}

void DynamicObjectImpl::setMember(const char* name, unsigned long long value)
{
   (*this)[name] = value;
}

void DynamicObjectImpl::setMember(const char* name, DynamicObject value)
{
   (*this)[name] = value;
}

const char* DynamicObjectImpl::getString(const char* name)
{
   // create string if necessary
   MemberValue* mv = &(*this)[name];
   if(mv->type == Object || mv->type == Array)
   {
      setMember(name, "");
   }
   
   return mv->str;
}

bool DynamicObjectImpl::getBoolean(const char* name)
{
   return (strcmp(getString(name), "true") == 0) ? true : false;
}

int DynamicObjectImpl::getInt32(const char* name)
{
   return strtol(getString(name), NULL, 10);
}

unsigned int DynamicObjectImpl::getUInt32(const char* name)
{
   return strtoul(getString(name), NULL, 10);
}

long long DynamicObjectImpl::getInt64(const char* name)
{
   return strtoll(getString(name), NULL, 10);
}

unsigned long long DynamicObjectImpl::getUInt64(const char* name)
{
   return strtoull(getString(name), NULL, 10);
}

DynamicObject DynamicObjectImpl::getObject(const char* name)
{
   // create object if necessary
   MemberValue* mv = &(*this)[name];
   if(mv->type != Object)
   {
      DynamicObject value;
      setMember(name, value);
   }
   
   return *mv->obj;
}

std::vector<DynamicObjectImpl::MemberValue>&
   DynamicObjectImpl::getArray(const char* name)
{
   // create array if necessary
   MemberValue* mv = &(*this)[name];
   if(mv->type != Array)
   {
      mv->freeData();
      mv->type = Array;
      mv->array = new std::vector<MemberValue>();
   }
   
   return *(*this)[name].array;
}
