/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/rt/DynamicObject.h"

#include "db/rt/DynamicObjectIterator.h"
#include "db/rt/DynamicObjectIterators.h"

#include <cstdlib>
#include <cctype>

using namespace db::rt;

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

bool DynamicObject::operator==(const DynamicObject& rhs) const
{
   bool rval;
   
   const DynamicObject& lhs = *this;
   rval = Collectable<DynamicObjectImpl>::operator==(rhs);
   if(!rval && !lhs.isNull() && !rhs.isNull())
   {
      // compare heap objects
      rval = (*lhs == *rhs);
   }
   
   return rval;
}

bool DynamicObject::operator!=(const DynamicObject& rhs) const
{
   return !(*this == rhs);
}

bool DynamicObject::operator<(const DynamicObject& rhs) const
{
   bool rval;
   
   const DynamicObject& lhs = *this;
   
   // NULL is always less than anything other than NULL
   if(lhs.isNull())
   {
      rval = !rhs.isNull();
   }
   // lhs is not NULL, but rhs is, so rhs is not less
   else if(rhs.isNull())
   {
      rval = false;
   }
   // neither lhs or rhs is NULL, compare heap objects
   else
   {
      rval = (*lhs < *rhs);
   }
   
   return rval;
}

void DynamicObject::operator=(const char* value)
{
   *mReference->ptr = value;
}

void DynamicObject::operator=(bool value)
{
   *mReference->ptr = value;
}

void DynamicObject::operator=(int32_t value)
{
   *mReference->ptr = value;
}

void DynamicObject::operator=(uint32_t value)
{
   *mReference->ptr = value;
}

void DynamicObject::operator=(int64_t value)
{
   *mReference->ptr = value;
}

void DynamicObject::operator=(uint64_t value)
{
   *mReference->ptr = value;
}

void DynamicObject::operator=(double value)
{
   *mReference->ptr = value;
}

DynamicObject& DynamicObject::operator[](const char* name)
{
   return (*mReference->ptr)[name];
}

DynamicObject& DynamicObject::operator[](int index)
{
   return (*mReference->ptr)[index];
}

DynamicObjectIterator DynamicObject::getIterator() const
{
   DynamicObjectIteratorImpl* i;

   switch((*this)->getType())
   {
      case Map:
         i = new DynamicObjectIteratorMap((DynamicObject&)*this);
         break;
      case Array:
         i = new DynamicObjectIteratorArray((DynamicObject&)*this);
         break;
      default:
         i = new DynamicObjectIteratorSingle((DynamicObject&)*this);
         break;
   }

   return DynamicObjectIterator(i);
}

DynamicObject DynamicObject::clone()
{
   DynamicObject rval;
   
   if(isNull())
   {
      rval.setNull();
   }
   else
   {
      int index = 0;
      rval->setType((*this)->getType());
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
   }
   
   return rval;
}

void DynamicObject::merge(DynamicObject& rhs, bool append)
{
   switch(rhs->getType())
   {
      case String:
      case Boolean:
      case Int32:
      case UInt32:
      case Int64:
      case UInt64:
      case Double:
         *this = rhs.clone();
         break;
      case Map:
      {
         (*this)->setType(Map);
         DynamicObjectIterator i = rhs.getIterator();
         while(i->hasNext())
         {
            DynamicObject& next = i->next();
            (*this)[i->getName()].merge(next, append);
         }
         break;
      }
      case Array:
         (*this)->setType(Array);
         DynamicObjectIterator i = rhs.getIterator();
         int offset = (append ? (*this)->length() : 0);
         for(int ii = 0; i->hasNext(); ii++)
         {
            (*this)[offset + ii].merge(i->next(), append);
         }
         break;
   }
}

bool DynamicObject::isSubset(const DynamicObject& rhs) const
{
   bool rval;
   
   rval = Collectable<DynamicObjectImpl>::operator==(rhs);
   if(!rval && (*this)->getType() == Map && rhs->getType() == Map)
   {
      // ensure right map has same or greater length
      if((*this)->length() <= rhs->length())
      {
         rval = true;
         DynamicObjectIterator i = this->getIterator();
         while(rval && i->hasNext())
         {
            DynamicObject& leftDyno = i->next();
            if(rhs->hasMember(i->getName()))
            {
               DynamicObject& rightDyno = (*rhs)[i->getName()];
               if(leftDyno->getType() == Map && rightDyno->getType() == Map)
               {
                  rval = leftDyno.isSubset(rightDyno);
               }
               else
               {
                  rval = (leftDyno == rightDyno);
               }
            }
            else
            {
               rval = false;
            }
         }
      }
   }
   
   return rval;
}

const char* DynamicObject::descriptionForType(DynamicObjectType type)
{
   const char* rval = NULL;
   
   switch(type)
   {
      case String:
         rval = "string";
         break;
      case Boolean:
         rval = "boolean";
         break;
      case Int32:
         rval = "32 bit integer";
         break;
      case UInt32:
         rval = "32 bit unsigned integer";
         break;
      case Int64:
         rval = "64 bit integer";
         break;
      case UInt64:
         rval = "64 bit unsigned integer";
         break;
      case Double:
         rval = "floating point";
         break;
      case Map:
         rval = "map";
         break;
      case Array:
         rval = "array";
         break;
   }
   
   return rval;
}

DynamicObjectType DynamicObject::determineType(const char* str)
{
   DynamicObjectType rval = String;
   
   // FIXME: this code might interpret hex/octal strings as integers
   // (and other code for that matter!) and we might not want to do that

   // if string starts with whitespace, forget about it
   if(!isspace(str[0]))
   {
      // see if the number is an unsigned int
      // then check signed int
      // then check doubles
      char* end;
      strtoull(str, &end, 10);
      if(end[0] == 0 && str[0] != '-')
      {
         // if end is NULL (and not negative) then the whole string was an int
         rval = UInt64;
      }
      else
      {
         // the number may be a signed int
         strtoll(str, &end, 10);
         if(end[0] == 0)
         {
            // if end is NULL then the whole string was an int
            rval = Int64;
         }
         else
         {
            // the number may be a double
            strtod(str, &end);
            if(end[0] == 0)
            {
               // end is NULL, so we've got a double,
               // else we've assume a String
               rval = Double;
            }
         }
      }
   }
   
   return rval;
}
