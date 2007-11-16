/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/util/DynamicObject.h"
#include "db/util/DynamicObjectIterator.h"

using namespace db::rt;
using namespace db::util;

DynamicObject::DynamicObject() :
   Collectable<DynamicObjectImpl>(new DynamicObjectImpl())
{
}

DynamicObject::DynamicObject(DynamicObjectImpl* impl) :
   Collectable<DynamicObjectImpl>(impl)
{
}

DynamicObject::DynamicObject(const DynamicObject& rhs) :
   Collectable<DynamicObjectImpl>(rhs)
{
}

DynamicObject::~DynamicObject()
{
}

bool DynamicObject::operator==(const DynamicObject& rhs)
{
   bool rval = false;
   
   DynamicObject* left = (DynamicObject*)this;
   DynamicObject* right = (DynamicObject*)&rhs;
   
   if(this == &rhs)
   {
      rval = true;
   }
   else if(this->mReference == rhs.mReference)
   {
      rval = true;
   }
   else if(this->mReference == NULL || rhs.mReference == NULL)
   {
      // one is NULL, other is not, so not equal
   }
   else if((*left)->getType() == (*right)->getType())
   {
      int index = 0;
      DynamicObjectIterator i;
      switch((*left)->getType())
      {
         case String:
            rval = (strcmp((*left)->getString(), (*right)->getString()) == 0);
            break;
         case Boolean:
            rval = (*left)->getBoolean() == (*right)->getBoolean();
            break;
         case Int32:
            rval = (*left)->getInt32() == (*right)->getInt32();
            break;
         case UInt32:
            rval = (*left)->getUInt32() == (*right)->getUInt32();
            break;
         case Int64:
            rval = (*left)->getInt64() == (*right)->getInt64();
            break;
         case UInt64:
            rval = (*left)->getUInt64() == (*right)->getUInt64();
            break;
         case Double:
            rval = (*left)->getDouble() == (*right)->getDouble();
            break;
         case Map:
            // ensure maps are the same length and contain the same entries
            if((*left)->length() == (*right)->length())
            {
               rval = true;
               i = left->getIterator();
               while(rval && i->hasNext())
               {
                  DynamicObject dyno = i->next();
                  if((*right)->hasMember(i->getName()))
                  {
                     rval = ((*right)[i->getName()] == dyno);
                  }
                  else
                  {
                     rval = false;
                  }
               }
            }
            break;
         case Array:
            // ensure arrays are the same length and contain the same elements
            // in the same order
            if((*left)->length() == (*right)->length())
            {
               rval = true;
               i = left->getIterator();
               while(rval && i->hasNext())
               {
                  rval = ((*right)[index++] == i->next());
               }
            }
            break;
      }
   }
   else
   {
      // compare based on string values
      std::string lstr, rstr;
      switch((*left)->getType())
      {
         case String:
         case Int32:
         case Int64:
         case UInt32:
         case UInt64:
         case Double:
            switch((*right)->getType())
            {
               case String:
               case Int32:
               case UInt32:
               case Int64:
               case UInt64:
               case Double:
                  (*left)->toString(lstr);
                  (*right)->toString(rstr);
                  rval = (strcmp(lstr.c_str(), rstr.c_str()) == 0);
                  break;
               default:
                  break;
            }
            break;
         default:
            break;
      }
   }
   
   return rval;
}

bool DynamicObject::operator!=(const DynamicObject& rhs)
{
   return !(*this == rhs);
}

void DynamicObject::operator=(const char* value)
{
   *mReference->ptr = value;
}

void DynamicObject::operator=(bool value)
{
   *mReference->ptr = value;
}

void DynamicObject::operator=(int value)
{
   *mReference->ptr = value;
}

void DynamicObject::operator=(unsigned int value)
{
   *mReference->ptr = value;
}

void DynamicObject::operator=(long long value)
{
   *mReference->ptr = value;
}

void DynamicObject::operator=(unsigned long long value)
{
   *mReference->ptr = value;
}

void DynamicObject::operator=(double value)
{
   *mReference->ptr = value;
}

DynamicObject& DynamicObject::operator[](const std::string& name)
{
   return (*mReference->ptr)[name];
}

DynamicObject& DynamicObject::operator[](int index)
{
   return (*mReference->ptr)[index];
}

DynamicObjectIterator DynamicObject::getIterator()
{
   return DynamicObjectIterator(new DynamicObjectIteratorImpl(*this));
}

DynamicObject DynamicObject::clone()
{
   DynamicObject rval;
   
   int index = 0;
   DynamicObjectIterator i = getIterator();
   while(i->hasNext())
   {
      DynamicObject dyno = i->next();
      switch((*this)->getType())
      {
         case String:
            rval = dyno->getString();
            break;
         case Boolean:
            rval = dyno->getBoolean();
            break;
         case Int32:
            rval = dyno->getInt32();
            break;
         case UInt32:
            rval = dyno->getUInt32();
            break;
         case Int64:
            rval = dyno->getInt64();
            break;
         case UInt64:
            rval = dyno->getUInt64();
            break;
         case Double:
            rval = dyno->getDouble();
            break;
         case Map:
            rval[i->getName()] = dyno.clone();
            break;
         case Array:
            rval[index++] = dyno.clone();
            break;
      }
   }
   
   return rval;
}
