/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/util/DynamicObjectImpl.h"
#include "db/util/DynamicObject.h"

using namespace db::util;

void DynamicObjectImpl::MemberValue::freeData()
{
   // clean up data based on type
   switch(type)
   {
      case Object:
         delete obj;
         break;
      case List:
         delete list;
         break;
      default:
         delete [] str;
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
      mvnew.type = String;
      mvnew.str = strdup("");
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
   return (*this)[name].str;
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
   return *(*this)[name].obj;
}
