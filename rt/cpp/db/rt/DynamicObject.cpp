/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/rt/DynamicObject.h"

#include "db/rt/DynamicObjectIterator.h"
#include "db/rt/DynamicObjectIterators.h"

#include <cstdlib>

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

bool DynamicObject::operator==(const DynamicObject& rhs)
{
   bool rval;
   
   DynamicObject* left = this;
   DynamicObject* right = (DynamicObject*)&rhs;
   
   rval = Collectable<DynamicObjectImpl>::operator==(rhs);
   if(!rval && !(*left).isNull() && !(*right).isNull())
   {
      if((*left)->getType() == (*right)->getType())
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
         switch((*left)->getType())
         {
            case String:
            case Boolean:
            case Int32:
            case Int64:
            case UInt32:
            case UInt64:
            case Double:
               switch((*right)->getType())
               {
                  case String:
                  case Boolean:
                  case Int32:
                  case UInt32:
                  case Int64:
                  case UInt64:
                  case Double:
                     rval = (strcmp(
                        (*left)->getString(),
                        (*right)->getString()) == 0);
                     break;
                  default:
                     break;
               }
               break;
            default:
               break;
         }
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

DynamicObjectIterator DynamicObject::getIterator()
{
   DynamicObjectIteratorImpl* i;

   switch((*this)->getType())
   {
      case Map:
         i = new DynamicObjectIteratorMap(*this);
         break;
      case Array:
         i = new DynamicObjectIteratorArray(*this);
         break;
      default:
         i = new DynamicObjectIteratorSingle(*this);
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
               DynamicObject next = i->next();
               (*this)[i->getName()].merge(next, append);
            }
         }
         break;
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

bool DynamicObject::isSubset(const DynamicObject& rhs)
{
   bool rval;
   
   DynamicObject* left = this;
   DynamicObject* right = (DynamicObject*)&rhs;
   
   rval = Collectable<DynamicObjectImpl>::operator==(rhs);
   if(!rval && (*left)->getType() == Map && (*right)->getType() == Map)
   {
      // ensure right map has same or greater length
      if((*left)->length() <= (*right)->length())
      {
         rval = true;
         DynamicObjectIterator i = left->getIterator();
         while(rval && i->hasNext())
         {
            DynamicObject leftDyno = i->next();
            if((*right)->hasMember(i->getName()))
            {
               DynamicObject rightDyno = (*right)[i->getName()];
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
   const char* rval;
   
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
