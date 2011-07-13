/*
 * Copyright (c) 2007-2011 Digital Bazaar, Inc. All rights reserved.
 */
#define __STDC_LIMIT_MACROS

#include "monarch/rt/DynamicObject.h"

#include "monarch/rt/DynamicObjectIterator.h"
#include "monarch/rt/DynamicObjectIterators.h"

#include <cstdlib>
#include <cctype>
#include <errno.h>
#include <limits.h>
#include <math.h>

using namespace monarch::rt;

DynamicObject::DynamicObject() :
   Collectable<DynamicObjectImpl>(new DynamicObjectImpl())
{
}

DynamicObject::DynamicObject(DynamicObjectType type) :
   Collectable<DynamicObjectImpl>(new DynamicObjectImpl())
{
   (*this)->setType(type);
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

bool DynamicObject::operator==(const char* rhs) const
{
   bool rval = false;

   const DynamicObject& lhs = *this;
   if(lhs.isNull() && rhs == NULL)
   {
      rval = true;
   }
   else if(!lhs.isNull() && rhs != NULL)
   {
      // compare heap object to string
      rval = (*lhs == rhs);
   }

   return rval;
}

bool DynamicObject::operator!=(const char* rhs) const
{
   return !(*this == rhs);
}

bool DynamicObject::operator<(const char* rhs) const
{
   bool rval = false;

   const DynamicObject& lhs = *this;
   if(!lhs.isNull() && rhs != NULL && lhs->getType() == String)
   {
      rval = strcmp(lhs->getString(), rhs) < 0;
   }

   return rval;
}

bool DynamicObject::operator<=(const char* rhs) const
{
   bool rval = false;

   const DynamicObject& lhs = *this;
   if(!lhs.isNull() && rhs != NULL && lhs->getType() == String)
   {
      rval = strcmp(lhs->getString(), rhs) <= 0;
   }

   return rval;
}

bool DynamicObject::operator>(const char* rhs) const
{
   bool rval = false;

   const DynamicObject& lhs = *this;
   if(!lhs.isNull() && rhs != NULL && lhs->getType() == String)
   {
      rval = strcmp(lhs->getString(), rhs) > 0;
   }

   return rval;
}

bool DynamicObject::operator>=(const char* rhs) const
{
   bool rval = false;

   const DynamicObject& lhs = *this;
   if(!lhs.isNull() && rhs != NULL && lhs->getType() == String)
   {
      rval = strcmp(lhs->getString(), rhs) >= 0;
   }

   return rval;
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

bool DynamicObject::operator>=(const DynamicObject& rhs) const
{
   return (*this == rhs) || (*this > rhs);
}

bool DynamicObject::operator>(const DynamicObject& rhs) const
{
   return (*this != rhs) && !(*this < rhs);
}

bool DynamicObject::operator<=(const DynamicObject& rhs) const
{
   return (*this == rhs) || (*this < rhs);
}

void DynamicObject::operator=(const char* value)
{
   *mReference->ptr = value;
}

void DynamicObject::operator=(const unsigned char* value)
{
   operator=((const char*)value);
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

DynamicObject::operator const char*() const
{
   return (*this)->getString();
}

DynamicObject::operator bool() const
{
   return (*this)->getBoolean();
}

DynamicObject::operator int32_t() const
{
   return (*this)->getInt32();
}

DynamicObject::operator uint32_t() const
{
   return (*this)->getUInt32();
}

DynamicObject::operator int64_t() const
{
   return (*this)->getInt64();
}

DynamicObject::operator uint64_t() const
{
   return (*this)->getUInt64();
}

DynamicObject::operator double() const
{
   return (*this)->getDouble();
}

DynamicObject& DynamicObject::operator[](char* name)
{
   return (*mReference->ptr)[name];
}

DynamicObject& DynamicObject::operator[](char* name) const
{
   return (*mReference->ptr)[name];
}

DynamicObject& DynamicObject::operator[](const char* name)
{
   return (*mReference->ptr)[name];
}

DynamicObject& DynamicObject::operator[](const char* name) const
{
   return (*mReference->ptr)[name];
}

DynamicObject& DynamicObject::operator[](unsigned char* name)
{
   return operator[]((const char*)name);
}

DynamicObject& DynamicObject::operator[](unsigned char* name) const
{
   return operator[]((const char*)name);
}

DynamicObject& DynamicObject::operator[](const unsigned char* name)
{
   return operator[]((const char*)name);
}

DynamicObject& DynamicObject::operator[](const unsigned char* name) const
{
   return operator[]((const char*)name);
}

DynamicObject& DynamicObject::operator[](int index)
{
   return (*mReference->ptr)[index];
}

DynamicObject& DynamicObject::operator[](int index) const
{
   return (*mReference->ptr)[index];
}

DynamicObjectIterator DynamicObject::getIterator() const
{
   DynamicObjectIteratorImpl* i;

   switch((*this)->getType())
   {
      case Map:
         i = new DynamicObjectIteratorMap(const_cast<DynamicObject&>(*this));
         break;
      case Array:
         i = new DynamicObjectIteratorArray(const_cast<DynamicObject&>(*this));
         break;
      default:
         i = new DynamicObjectIteratorSingle(const_cast<DynamicObject&>(*this));
         break;
   }

   return i;
}

DynamicObject& DynamicObject::push(DynamicObject value)
{
   // for non-arrays, preserve original value if set
   if((*this)->getType() != Array && !(*this)->isUnset())
   {
      DynamicObject d;
      d->append(*this);
      operator=(d);
   }
   (*this)->append(value);
   return *this;
}

DynamicObject& DynamicObject::push(const char* value)
{
   DynamicObject d;
   d = value;
   return push(d);
}

DynamicObject& DynamicObject::push(bool value)
{
   DynamicObject d;
   d = value;
   return push(d);
}

DynamicObject& DynamicObject::push(int32_t value)
{
   DynamicObject d;
   d = value;
   return push(d);
}

DynamicObject& DynamicObject::push(uint32_t value)
{
   DynamicObject d;
   d = value;
   return push(d);
}

DynamicObject& DynamicObject::push(int64_t value)
{
   DynamicObject d;
   d = value;
   return push(d);
}

DynamicObject& DynamicObject::push(uint64_t value)
{
   DynamicObject d;
   d = value;
   return push(d);
}

DynamicObject& DynamicObject::push(double value)
{
   DynamicObject d;
   d = value;
   return push(d);
}

DynamicObject DynamicObject::pop()
{
   return (*this)->pop();
}

DynamicObject DynamicObject::shift()
{
   DynamicObject rval(NULL);
   if((*this)->getType() == Array)
   {
      DynamicObjectIterator i = getIterator();
      if(i->hasNext())
      {
         rval = i->next();
         i->remove();
      }
   }
   return rval;
}

DynamicObject DynamicObject::first() const
{
   DynamicObject rval(NULL);

   switch((*this)->getType())
   {
      case String:
      case Boolean:
      case Int32:
      case UInt32:
      case Int64:
      case UInt64:
      case Double:
         rval = *this;
         break;
      case Map:
      {
         // return first result of iterator
         DynamicObjectIterator i = getIterator();
         if(i->hasNext())
         {
            rval = i->next();
         }
         break;
      }
      case Array:
      {
         if((*this)->length() > 0)
         {
            rval = (*this)[0];
         }
         break;
      }
   }

   return rval;
}

DynamicObject DynamicObject::last() const
{
   DynamicObject rval(NULL);

   switch((*this)->getType())
   {
      case String:
      case Boolean:
      case Int32:
      case UInt32:
      case Int64:
      case UInt64:
      case Double:
         rval = *this;
         break;
      case Map:
      {
         // return last result of iterator
         // FIXME: inefficient, need reverse iterators
         DynamicObjectIterator i = getIterator();
         while(i->hasNext())
         {
            rval = i->next();
         }
         break;
      }
      case Array:
      {
         int len = (*this)->length();
         if(len > 0)
         {
            rval = (*this)[len - 1];
         }
         break;
      }
   }

   return rval;
}

DynamicObject DynamicObject::keys()
{
   DynamicObject rval(Array);
   if((*this)->getType() == Map)
   {
      DynamicObjectIterator i = this->getIterator();
      while(i->hasNext())
      {
         i->next();
         rval->append(i->getName());
      }
   }
   return rval;
}

DynamicObject DynamicObject::values()
{
   DynamicObject rval(Array);
   if((*this)->getType() == Map)
   {
      DynamicObjectIterator i = this->getIterator();
      while(i->hasNext())
      {
         rval->append(i->next());
      }
   }
   return rval;
}

DynamicObject& DynamicObject::sort(DynamicObject::CompareLessDyno func)
{
   if((*this)->getType() == Array)
   {
      if(func == NULL)
      {
         // use default operator<()
         std::sort((*this)->mArray->begin(), (*this)->mArray->end());
      }
      else
      {
         // use function
         std::sort((*this)->mArray->begin(), (*this)->mArray->end(), func);
      }
   }
   return *this;
}

/**
 * A simple structure used to wrap a DynamicObject::SortFunctor because
 * the stl sort function will copy the SortFunctor and this behavior might
 * not have been coded for w/copy constructors, etc.
 */
struct _SortDyno
{
   DynamicObject::SortFunctor& functor;
   _SortDyno(DynamicObject::SortFunctor& f) :
      functor(f) {}
   bool operator()(const DynamicObject& a, const DynamicObject& b) const
   {
      return functor(
         const_cast<DynamicObject&>(a),
         const_cast<DynamicObject&>(b));
   }
};

DynamicObject& DynamicObject::sort(DynamicObject::SortFunctor& func)
{
   if((*this)->getType() == Array)
   {
      std::sort(
         (*this)->mArray->begin(), (*this)->mArray->end(), _SortDyno(func));
   }
   return *this;
}

DynamicObject DynamicObject::filter(DynamicObject::FilterDyno func)
{
   DynamicObject rval(Array);
   if((*this)->getType() == Array)
   {
      DynamicObjectIterator i = this->getIterator();
      while(i->hasNext())
      {
         DynamicObject& next = i->next();
         if(func(next))
         {
            rval->append(next);
         }
      }
   }
   return rval;
}

DynamicObject DynamicObject::filter(DynamicObject::FilterFunctor& func)
{
   DynamicObject rval(Array);
   if((*this)->getType() == Array)
   {
      DynamicObjectIterator i = this->getIterator();
      while(i->hasNext())
      {
         DynamicObject& next = i->next();
         if(func(next))
         {
            rval->append(next);
         }
      }
   }
   return rval;
}

DynamicObject& DynamicObject::rotate(int num, bool left)
{
   int length = (*this)->length();
   if((*this)->getType() == Array && length > 1 && num > 0)
   {
      // make a shallow copy of the array
      DynamicObject copy(Array);
      copy.merge(*this, true);

      // rotate the elements
      num %= length;
      if(!left)
      {
         num = -num;
      }
      for(int i = 0; i < length; ++i)
      {
         (*this)[i] = copy[(i + num + length) % length];
      }
   }
   return *this;
}

DynamicObject DynamicObject::slice(int start, int end)
{
   DynamicObject rval(Array);

   int length = (*this)->length();
   if(end == -1)
   {
      end = length;
   }
   if((*this)->getType() == Array && length > 0 && start < end)
   {
      // force slice to be within bounds
      if(start < 0)
      {
         start = 0;
      }
      if(end > length)
      {
         end = length;
      }

      for(int i = start; i < end; ++i)
      {
         rval->append((*this)[i]);
      }
   }

   return rval;
}

DynamicObject DynamicObject::clone()
{
   DynamicObject rval(NULL);

   if(!isNull())
   {
      DynamicObjectType type = (*this)->getType();
      rval = DynamicObject(type);
      int index = 0;
      DynamicObjectIterator i = getIterator();
      while(i->hasNext())
      {
         DynamicObject& dyno = i->next();
         switch(type)
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

/**
 * The _getMapDiff helper function gets the differences between the source
 * object and the target object and places the result in the result object.
 * The comparison flags are passed to the diffing algorithm.
 *
 * If differences are found, the result object will be an array with the
 * differences.
 * [
 *    {
 *       "key": <key>
 *       "<type>": <value>
 *    },
 *    ...
 * ]
 * Where <key> is the source or target key, <type> is either
 * "added", "removed", or "changed", and value is the added or
 * removed value or a sub-diff result.
 *
 * @param source the source object to compare against the target object.
 * @param target the target object that will be compared against the source
 *               object.
 * @param result the result of the diffing operation.
 * @param flags the flags that will be passed to the recursive diffing
 *              operation.
 *
 * @return true if there are differences, false otherwise.
 */
static bool _getMapDiff(
   DynamicObject& source, DynamicObject& target, DynamicObject& result,
   uint32_t flags)
{
   bool rval = false;

   // keep track of keys we've checked
   DynamicObject checked(Map);

   // Check all the source keys
   DynamicObjectIterator i = source.getIterator();
   while(i->hasNext())
   {
      DynamicObject& next = i->next();
      const char* name = i->getName();
      checked[name] = true;
      if(target->hasMember(name))
      {
         // recusively get sub-diff
         DynamicObject d;
         if(next.diff(target[name], d, flags))
         {
            // diff found, add it
            if(!rval)
            {
               // first diff, init result array
               result->setType(Array);
               rval = true;
            }
            rval = true;
            DynamicObject& change = result->append();
            change["key"] = name;
            change["changed"] = d;
         }
      }
      else
      {
         // key removed from target
         if(!rval)
         {
            // first diff, init result array
            result->setType(Array);
            rval = true;
         }
         DynamicObject& change = result->append();
         change["key"] = name;
         change["removed"] = next.clone();
      }
   }

   // Check for added target keys
   DynamicObjectIterator ti = target.getIterator();
   while(ti->hasNext())
   {
      DynamicObject& next = ti->next();
      const char* name = ti->getName();
      if(!checked->hasMember(name))
      {
         if(!rval)
         {
            // first diff, init result array
            result->setType(Array);
            rval = true;
         }
         DynamicObject& change = result->append();
         change["key"] = name;
         change["added"] = next.clone();
      }
   }

   return rval;
}

/**
 * The _getArrayDiff helper function gets the differences between the source
 * object and the target object and places the result in the result object.
 * The comparison flags are passed to the diffing algorithm.
 *
 * If differences are found, the result object will be an array with the
 * differences.
 * [
 *    {
 *       "index": <index>
 *       "<type>": <value>
 *    },
 *    ...
 * ]
 * Where <index> is the source or target index, <type> is either
 * "added", "removed", or "changed", and value is the added or
 * removed value or a sub-diff result.
 *
 * @param source the source object to compare against the target object.
 * @param target the target object that will be compared against the source
 *               object.
 * @param result the result of the diffing operation.
 * @param flags the flags that will be passed to the recursive diffing
 *              operation.
 *
 * @return true if there are differences, false otherwise.
 */
static bool _getArrayDiff(
   DynamicObject& source, DynamicObject& target, DynamicObject& result,
   uint32_t flags)
{
   bool rval = false;
   int i;

   // check for differences and removals
   for(i = 0; i < source->length(); ++i)
   {
      if(i >= target->length())
      {
         // we are beyond target length so items were removed
         // check if first
         if(!rval)
         {
            // first diff, init result array
            result->setType(Array);
            rval = true;
         }
         DynamicObject& change = result->append();
         change["index"] = i;
         change["removed"] = source[i].clone();
      }
      else
      {
         // check if items are the same
         DynamicObject diff;
         if(source[i].diff(target[i], diff, flags))
         {
            // diff found, check if first
            if(!rval)
            {
               // first diff, init result array
               result->setType(Array);
               rval = true;
            }
            DynamicObject& change = result->append();
            change["index"] = i;
            change["changed"] = diff;
         }
      }
   }

   // check for additions
   for(; i < target->length(); ++i)
   {
      // we are beyond source length so items were added
      // check if first
      if(!rval)
      {
         // first diff, init result array
         result->setType(Array);
         rval = true;
      }
      DynamicObject& added = result->append();
      added["index"] = i;
      added["added"] = target[i];
   }

   return rval;
}

bool DynamicObject::diff(
   DynamicObject& target, DynamicObject& result, uint32_t flags)
{
   bool rval = false;

   DynamicObject& source = (DynamicObject&)*this;
   // clear any old values
   result->clear();

   // flags for simple (ie, non-map and non-array) changes
   bool hasTypeChange = false;
   bool hasValueChange = false;

   bool snull = source.isNull();
   bool tnull = target.isNull();

   if(snull && tnull)
   {
      // same: no diff
   }
   else if((!snull && tnull) || (snull && !tnull))
   {
      // source or target is null but other is not
      hasTypeChange = true;
   }
   else
   {
      // not null && same type: diff=deep compare
      switch(source->getType())
      {
         case Int32:
         case Int64:
         case UInt32:
         case UInt64:
            if(source->getType() != target->getType())
            {
               switch(target->getType())
               {
                  case Int32:
                  case Int64:
                  case UInt32:
                  case UInt64:
                     // if we're comparing using 64 bit integers, ignore
                     // differences between 32 bit and 64 bit integers
                     if(flags & DiffIntegersAsInt64s)
                     {
                        // do signed comparison
                        if(source->getType() == Int32 ||
                           source->getType() == Int64)
                        {
                           if(source->getInt64() != target->getInt64())
                           {
                              hasValueChange = true;
                           }
                        }
                        // do unsigned comparison
                        else if(source->getUInt64() != target->getUInt64())
                        {
                           hasValueChange = true;
                        }

                        // only break out of case if we're comparing
                        // using 64 bit integers, otherwise drop to the
                        // default case of a type mismatch
                        break;
                     }
                  default:
                     hasTypeChange = true;
               }
            }
            else if(source != target)
            {
               hasValueChange = true;
            }
            break;
         case Double:
            // compare doubles as strings if requested
            if((flags & DiffDoublesAsStrings) &&
               target->getType() == Double)
            {
               if(strcmp(source->getString(), target->getString()) != 0)
               {
                  hasValueChange = true;
               }
               // only break out of case if comparing as strings
               break;
            }
         case String:
         case Boolean:
            if(source->getType() != target->getType())
            {
               hasTypeChange = true;
            }
            else if(source != target)
            {
               hasValueChange = true;
            }
            break;
         case Map:
            if(source->getType() != target->getType())
            {
               hasTypeChange = true;
            }
            else
            {
               // get the Map differences between source and target
               rval = _getMapDiff(source, target, result, flags);
            }
            break;
         case Array:
            if(source->getType() != target->getType())
            {
               hasTypeChange = true;
            }
            else
            {
               // get the Array differences between source and target
               rval = _getArrayDiff(source, target, result, flags);
            }
            break;
      }
   }

   if(hasTypeChange || hasValueChange)
   {
      rval = true;
      result["type"] = hasTypeChange ? "typeChanged" : "valueChanged";
      result["source"] = source;
      result["target"] = target;
   }

   return rval;
}

void DynamicObject::merge(DynamicObject& rhs, bool append)
{
   if(!rhs.isNull())
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
         {
            // append if lhs is array (and append flag is true)
            if(append && !this->isNull() && (*this)->getType() == Array)
            {
               (*this)->append() = rhs.clone();
            }
            else
            {
               *this = rhs.clone();
            }
            break;
         }
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
            for(int ii = 0; i->hasNext(); ++ii)
            {
               (*this)[offset + ii].merge(i->next(), append);
            }
            break;
      }
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
      // if the string has no decimal point, it might be an integer
      if(strchr(str, '.') == NULL)
      {
         // see if the number is an unsigned int
         // then check signed int
         char* end;
         errno = 0;
         uint64_t ui64 = strtoull(str, &end, 10);
         if(end[0] == 0 && str[0] != '-' && (ui64 != ULLONG_MAX || errno == 0))
         {
            // if end is NULL (and not negative) then the whole string was
            // an int
            rval = UInt64;
         }
         else
         {
            // the number may be a signed int
            errno = 0;
            int64_t i64 = strtoll(str, &end, 10);
            if(end[0] == 0 &&
               ((i64 != LLONG_MAX && i64 != LLONG_MIN) || errno == 0))
            {
               // if end is NULL then the whole string was an int
               rval = Int64;
            }
         }
      }

      // integer not detected, check double
      if(rval == String)
      {
         char* end;
         errno = 0;
         double d = strtod(str, &end);
         if(end[0] == 0 && ((d != HUGE_VAL && d != -HUGE_VAL) || errno == 0))
         {
            // end is NULL, so we've got a double,
            // else we've assume a String
            rval = Double;
         }
      }
   }

   return rval;
}

bool operator==(const char* lhs, const DynamicObject& rhs)
{
   return rhs == lhs;
}

bool operator!=(const char* lhs, const DynamicObject& rhs)
{
   return rhs != lhs;
}

bool operator<(const char* lhs, const DynamicObject& rhs)
{
   return rhs > lhs;
}

bool operator<=(const char* lhs, const DynamicObject& rhs)
{
   return rhs >= lhs;
}

bool operator>(const char* lhs, const DynamicObject& rhs)
{
   return rhs < lhs;
}

bool operator>=(const char* lhs, const DynamicObject& rhs)
{
   return rhs <= lhs;
}
