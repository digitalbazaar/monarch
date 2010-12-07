/*
 * Copyright (c) 2007-2010 Digital Bazaar, Inc. All rights reserved.
 */
#define __STDC_LIMIT_MACROS
#define __STDC_FORMAT_MACROS

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "monarch/rt/DynamicObjectImpl.h"

#include "monarch/rt/DynamicObject.h"
#include "monarch/util/Macros.h"
#ifdef MO_DYNO_KEY_COUNTS
#include "monarch/rt/ExclusiveLock.h"
#endif

#include <cstdlib>
#include <cstdio>
#include <string>
#include <algorithm>

using namespace std;
using namespace monarch::rt;

#if defined(MO_DYNO_COUNTS) || defined(MO_DYNO_KEY_COUNTS)

/**
 * When MO_DYNO_*COUNTS is defined at compile time statistics will be kept on
 * internal DynamicObject operations. This may have a performance impact.
 * Some stats such as basic counts are atomic. However, some secondary stats
 * such as maximums are not calculated in a thread safe way due to
 * performance issues. Also note that the entire stats structure can not be
 * syncronized so clearing and reading of the structure may be slightly
 * inaccurate if other threads are asynchronously updating the stats.
 */
struct _stats_data_s
{
   struct {
      uint64_t live;
      uint64_t dead;
      uint64_t max;
   } counts;
   struct {
      uint64_t live;
      uint64_t dead;
      uint64_t max;
   } bytes;
};

static bool _stats_enabled;

#define _STATS_INC(stat) \
   MO_STMT_START { \
      if(_stats_enabled) \
      { \
         uint64_t next = \
            __sync_add_and_fetch(&(stat)->counts.live, 1); \
         if(next > (stat)->counts.max) \
         { \
            (stat)->counts.max = next; \
         } \
      } \
   } MO_STMT_END

#define _STATS_BYTES_INC(stat, n) \
   MO_STMT_START { \
      if(_stats_enabled) \
      { \
         uint64_t next = \
            __sync_add_and_fetch(&(stat)->bytes.live, n); \
         if(next > (stat)->bytes.max) \
         { \
            (stat)->bytes.max = next; \
         } \
      } \
   } MO_STMT_END

#define _STATS_DEC(stat) \
   MO_STMT_START { \
      if(_stats_enabled) \
      { \
         __sync_sub_and_fetch(&(stat)->counts.live, 1); \
         __sync_add_and_fetch(&(stat)->counts.dead, 1); \
      } \
   } MO_STMT_END

#define _STATS_BYTES_DEC(stat, n) \
   MO_STMT_START { \
      if(_stats_enabled) \
      { \
         __sync_sub_and_fetch(&(stat)->bytes.live, n); \
         __sync_add_and_fetch(&(stat)->bytes.dead, n); \
      } \
   } MO_STMT_END

#endif // MO_DYNO_COUNTS || MO_DYNO_KEY_COUNTS */

#ifdef MO_DYNO_COUNTS

enum {
   Object = LastDynamicObjectType + 1,
   Key,
   StringValue,
   LastStatsType
};

static struct _stats_data_s _stats_counts[LastStatsType];

#define STATS_COUNTS_INC(type) \
   _STATS_INC(&_stats_counts[(type)]);
#define STATS_COUNTS_BYTES_INC(type, n) \
   _STATS_BYTES_INC(&_stats_counts[(type)], (n));
#define STATS_COUNTS_DEC(type) \
   _STATS_DEC(&_stats_counts[(type)]);
#define STATS_COUNTS_BYTES_DEC(type, n) \
   _STATS_BYTES_DEC(&_stats_counts[(type)], (n));

#else

// do debugging
#define STATS_COUNTS_INC(type)
#define STATS_COUNTS_BYTES_INC(type, n)
#define STATS_COUNTS_DEC(type)
#define STATS_COUNTS_BYTES_DEC(type, n)

#endif // MO_DYNO_COUNTS

#ifdef MO_DYNO_COUNTS
#define _changeType(dyno, newType) \
   MO_STMT_START { \
      STATS_COUNTS_DEC((dyno)->mType); \
      dyno->mType = (newType); \
      STATS_COUNTS_INC((newType)); \
   } MO_STMT_END
#else
#define _changeType(dyno, newType) \
   MO_STMT_START { \
      (dyno)->mType = (newType); \
   } MO_STMT_END
#endif // MO_DYNO_COUNTS

#ifdef MO_DYNO_KEY_COUNTS

typedef std::map<const char*, struct _stats_data_s,
   DynamicObjectImpl::MemberComparator>
   _StatsKeyMap;
static _StatsKeyMap _stats_key_counts;
static monarch::rt::ExclusiveLock _stats_key_counts_lock;

#define STATS_KEY_COUNTS_INC(key) \
   MO_STMT_START { \
      if(_stats_enabled) \
      { \
         struct _stats_data_s* s; \
         _stats_key_counts_lock.lock(); \
         if(_stats_key_counts.count((key)) == 0) \
         { \
            s = &_stats_key_counts[strdup((key))]; \
         } \
         else \
         { \
            s = &_stats_key_counts[(key)]; \
         } \
         _stats_key_counts_lock.unlock(); \
         _STATS_INC(s); \
      } \
   } MO_STMT_END

#define STATS_KEY_COUNTS_BYTES_INC(key, n) \
   MO_STMT_START { \
      if(_stats_enabled) \
      { \
         struct _stats_data_s* s; \
         _stats_key_counts_lock.lock(); \
         if(_stats_key_counts.count((key)) == 0) \
         { \
            s = &_stats_key_counts[strdup((key))]; \
         } \
         else \
         { \
            s = &_stats_key_counts[(key)]; \
         } \
         _stats_key_counts_lock.unlock(); \
         _STATS_BYTES_INC(s, (n)); \
      } \
   } MO_STMT_END

#define STATS_KEY_COUNTS_DEC(key) \
   MO_STMT_START { \
      if(_stats_enabled) \
      { \
         struct _stats_data_s* s; \
         _stats_key_counts_lock.lock(); \
         s = &_stats_key_counts[(key)]; \
         _stats_key_counts_lock.unlock(); \
         _STATS_DEC(s); \
      } \
   } MO_STMT_END

#define STATS_KEY_COUNTS_BYTES_DEC(key, n) \
   MO_STMT_START { \
      if(_stats_enabled) \
      { \
         struct _stats_data_s* s; \
         _stats_key_counts_lock.lock(); \
         s = &_stats_key_counts[(key)]; \
         _stats_key_counts_lock.unlock(); \
         _STATS_BYTES_DEC(s, (n)); \
      } \
   } MO_STMT_END

#else

#define STATS_KEY_COUNTS_INC(key)
#define STATS_KEY_COUNTS_BYTES_INC(key, n)
#define STATS_KEY_COUNTS_DEC(key)
#define STATS_KEY_COUNTS_BYTES_DEC(key, n)

#endif // MO_DYNO_KEY_COUNTS

DynamicObjectImpl::DynamicObjectImpl()
{
   STATS_COUNTS_INC(Object);
   STATS_COUNTS_INC(String);

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
      STATS_COUNTS_DEC(StringValue);
      STATS_COUNTS_BYTES_DEC(StringValue, strlen(mStringValue));
      free(mStringValue);
   }

   STATS_COUNTS_DEC(mType);
   STATS_COUNTS_DEC(Object);
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
         for(; i != value.mMap->end(); ++i)
         {
            // create new map entry
            STATS_COUNTS_INC(Key);
            STATS_COUNTS_BYTES_INC(Key, strlen(i->first));
            STATS_KEY_COUNTS_INC(i->first);
            STATS_KEY_COUNTS_BYTES_INC(i->first, strlen(i->first));
            mMap->insert(std::make_pair(strdup(i->first), i->second));
         }
         break;
      }
      case Array:
      {
         setType(Array);
         clear();
         ObjectArray::iterator i = value.mArray->begin();
         for(; i != value.mArray->end(); ++i)
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
   _changeType(this, String);
   mString = strdup(value);
   STATS_COUNTS_INC(String);
   STATS_COUNTS_BYTES_INC(String, strlen(value));
}

void DynamicObjectImpl::operator=(bool value)
{
   freeData();
   _changeType(this, Boolean);
   mBoolean = value;
}

void DynamicObjectImpl::operator=(int32_t value)
{
   freeData();
   _changeType(this, Int32);
   mInt32 = value;
}

void DynamicObjectImpl::operator=(uint32_t value)
{
   freeData();
   _changeType(this, UInt32);
   mUInt32 = value;
}

void DynamicObjectImpl::operator=(int64_t value)
{
   freeData();
   _changeType(this, Int64);
   mInt64 = value;
}

void DynamicObjectImpl::operator=(uint64_t value)
{
   freeData();
   _changeType(this, UInt64);
   mUInt64 = value;
}

void DynamicObjectImpl::operator=(double value)
{
   freeData();
   _changeType(this, Double);
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
      STATS_COUNTS_INC(Key);
      STATS_COUNTS_BYTES_INC(Key, strlen(name));
      STATS_KEY_COUNTS_INC(name);
      STATS_KEY_COUNTS_BYTES_INC(name, strlen(name));
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
      for(; i > 0; --i)
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
   bool rval = false;

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
               for(; rval && li != mMap->end(); ++li, ++ri)
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

bool DynamicObjectImpl::operator==(const char* rhs) const
{
   bool rval = false;
   if(mType == String)
   {
      rval = (strcmp(getString(), rhs) == 0);
   }
   return rval;
}

bool DynamicObjectImpl::operator<(const DynamicObjectImpl& rhs) const
{
   bool rval = false;

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
               for(; !rval && li != mMap->end(); ++li, ++ri)
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
            rval = (*mArray < *(rhs.mArray));
            break;
      }
   }
   else
   {
      // FIXME: make isNumber() and isNumeric() functions?

      // determine if lhs is a number and is negative
      bool lhsDouble = (mType == Double);
      bool lhsNumber = false;
      bool lhsNegative = false;
      if(mType == Int32 || mType == Int64 || lhsDouble)
      {
         lhsNegative = getDouble() < 0;
         lhsNumber = true;
      }
      else if(mType == UInt32 || mType == UInt64)
      {
         lhsNumber = true;
      }

      // determine if rhs is a number and is negative
      bool rhsDouble = (rhs.mType == Double);
      bool rhsNumber = false;
      bool rhsNegative = false;
      if(rhs.mType == Int32 || rhs.mType == Int64 || rhsDouble)
      {
         rhsNegative = getDouble() < 0;
         rhsNumber = true;
      }
      else if(rhs.mType == UInt32 || rhs.mType == UInt64)
      {
         rhsNumber = true;
      }

      // do number comparison if both lhs and rhs are numbers
      if(lhsNumber && rhsNumber)
      {
         if(lhsNegative == rhsNegative)
         {
            if(lhsDouble || rhsDouble)
            {
               rval = getDouble() < rhs.getDouble();
            }
            else if(lhsNegative || rhsNegative)
            {
               rval = getInt64() < rhs.getInt64();
            }
            else
            {
               rval = getUInt64() < rhs.getUInt64();
            }
         }
         else if(lhsNegative)
         {
            // lhs is negative, rhs is not
            rval = true;
         }
         else
         {
            // rhs is negative, lhs is not
            rval = false;
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
   }

   return rval;
}

DynamicObject& DynamicObjectImpl::append()
{
   setType(Array);
   DynamicObject d;
   mArray->push_back(d);
   return mArray->back();
}

DynamicObject& DynamicObjectImpl::append(DynamicObject& value)
{
   setType(Array);
   mArray->push_back(value);
   return mArray->back();
}

DynamicObject& DynamicObjectImpl::append(const char* value)
{
   DynamicObject dyno;
   dyno = value;
   setType(Array);
   mArray->push_back(dyno);
   return mArray->back();
}

DynamicObject& DynamicObjectImpl::append(bool value)
{
   DynamicObject dyno;
   dyno = value;
   setType(Array);
   mArray->push_back(dyno);
   return mArray->back();
}

DynamicObject& DynamicObjectImpl::append(int32_t value)
{
   DynamicObject dyno;
   dyno = value;
   setType(Array);
   mArray->push_back(dyno);
   return mArray->back();
}

DynamicObject& DynamicObjectImpl::append(uint32_t value)
{
   DynamicObject dyno;
   dyno = value;
   setType(Array);
   mArray->push_back(dyno);
   return mArray->back();
}

DynamicObject& DynamicObjectImpl::append(int64_t value)
{
   DynamicObject dyno;
   dyno = value;
   setType(Array);
   mArray->push_back(dyno);
   return mArray->back();
}

DynamicObject& DynamicObjectImpl::append(uint64_t value)
{
   DynamicObject dyno;
   dyno = value;
   setType(Array);
   mArray->push_back(dyno);
   return mArray->back();
}

DynamicObject& DynamicObjectImpl::append(double value)
{
   DynamicObject dyno;
   dyno = value;
   setType(Array);
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
            _changeType(this, Map);
            mMap = new ObjectMap();
            break;
         }
         case Array:
         {
            freeData();
            _changeType(this, Array);
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
   else if(mType == Map)
   {
      // return blank string
      rval = "";
   }
   else if(mType == Array)
   {
      // return blank string
      if(mArray->size() == 0)
      {
         rval = "";
      }
      // return first element of array as a string
      else
      {
         rval = (*mArray)[0]->getString();
      }
   }
   else
   {
      char* str = (char*)(mStringValue);
      if(str == NULL)
      {
         STATS_COUNTS_INC(StringValue);
      }
      else
      {
         STATS_COUNTS_BYTES_DEC(StringValue, strlen(str));
      }

      // convert type as appropriate
      switch(mType)
      {
         case Boolean:
            str = (char*)realloc(str, 6);
            snprintf(str, 6, "%s", (mBoolean ? "true" : "false"));
            break;
         case Int32:
            str = (char*)realloc(str, 12);
            snprintf(str, 12, "%" PRIi32, mInt32);
            break;
         case UInt32:
            str = (char*)realloc(str, 11);
            snprintf(str, 11, "%" PRIu32, mUInt32);
            break;
         case Int64:
            str = (char*)realloc(str, 22);
            snprintf(str, 22, "%" PRIi64, mInt64);
            break;
         case UInt64:
            str = (char*)realloc(str, 21);
            snprintf(str, 21, "%" PRIu64, mUInt64);
            break;
         case Double:
            // use default precision of 6
            // X.000000e+00 = 11 places to right of decimal
            str = (char*)realloc(str, 50);
            snprintf(str, 50, "%e", mDouble);
            break;
         default: /* String, Map, Array, ... already handled */
            break;
      }

      STATS_COUNTS_BYTES_INC(StringValue, strlen(str));

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

int DynamicObjectImpl::indexOf(const char* value) const
{
   DynamicObject d;
   d = value;
   return indexOf(d);
}

int DynamicObjectImpl::indexOf(bool value) const
{
   DynamicObject d;
   d = value;
   return indexOf(d);
}

int DynamicObjectImpl::indexOf(int32_t value) const
{
   DynamicObject d;
   d = value;
   return indexOf(d);
}

int DynamicObjectImpl::indexOf(uint32_t value) const
{
   DynamicObject d;
   d = value;
   return indexOf(d);
}

int DynamicObjectImpl::indexOf(int64_t value) const
{
   DynamicObject d;
   d = value;
   return indexOf(d);
}

int DynamicObjectImpl::indexOf(uint64_t value) const
{
   DynamicObject d;
   d = value;
   return indexOf(d);
}

int DynamicObjectImpl::indexOf(double value) const
{
   DynamicObject d;
   d = value;
   return indexOf(d);
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
   int rval = -1;

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
   for(ObjectMap::iterator i = mMap->begin(); i != mMap->end(); ++i)
   {
      char* key = (char*)i->first;
      STATS_COUNTS_DEC(Key);
      STATS_COUNTS_BYTES_DEC(Key, strlen(key));
      STATS_KEY_COUNTS_DEC(key);
      STATS_KEY_COUNTS_BYTES_DEC(key, strlen(key));
      free(key);
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
            STATS_COUNTS_BYTES_DEC(String, strlen(mString));
            free(mString);
            mString = NULL;
         }
         break;
      case Map:
         if(mMap != NULL)
         {
            freeMapKeys();
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

void DynamicObjectImpl::removeMember(ObjectMap::iterator iterator)
{
   // clean up key and remove map entry
   char* key = (char*)iterator->first;
   STATS_COUNTS_DEC(Key);
   STATS_COUNTS_BYTES_DEC(Key, strlen(key));
   STATS_KEY_COUNTS_DEC(key);
   STATS_KEY_COUNTS_BYTES_DEC(key, strlen(key));
   free(key);
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

bool DynamicObjectImpl::enableStats(bool enable)
{
#ifdef MO_DYNO_COUNTS
   bool rval = _stats_enabled;
   _stats_enabled = enable;
#else
   bool rval = false;
#endif
   return rval;
}

void DynamicObjectImpl::clearStats()
{
#ifdef MO_DYNO_COUNTS
   memset(&_stats_counts, 0, sizeof(_stats_counts));
#endif
#ifdef MO_DYNO_KEY_COUNTS
   _stats_key_counts_lock.lock();
   _stats_key_counts.clear();
   _stats_key_counts_lock.unlock();
#endif
}

DynamicObject DynamicObjectImpl::getStats()
{
   DynamicObject rval;
   rval->setType(Map);

#if defined(MO_DYNO_COUNTS) || defined(MO_DYNO_KEY_COUNTS)
   #define MAKESTAT(d, s) \
      MO_STMT_START { \
         d["counts"]["live"] = s.counts.live; \
         d["counts"]["dead"] = s.counts.dead; \
         d["counts"]["max"] = s.counts.max; \
         d["bytes"]["live"] = s.bytes.live; \
         d["bytes"]["dead"] = s.bytes.dead; \
         d["bytes"]["max"] = s.bytes.max; \
      } MO_STMT_END
#endif // defined(MO_DYNO_COUNTS) || defined(MO_DYNO_KEY_COUNTS)

#ifdef MO_DYNO_COUNTS
   #define GETSTAT(s, type) \
      MO_STMT_START { \
         DynamicObject& d = s[MO_STRINGIFY(type)]; \
         MAKESTAT(d, _stats_counts[type]); \
      } MO_STMT_END
   GETSTAT(rval, Object);
   GETSTAT(rval, String);
   GETSTAT(rval, Boolean);
   GETSTAT(rval, Int32);
   GETSTAT(rval, UInt32);
   GETSTAT(rval, Int64);
   GETSTAT(rval, UInt64);
   GETSTAT(rval, Double);
   GETSTAT(rval, Map);
   GETSTAT(rval, Array);
   GETSTAT(rval, Key);
   GETSTAT(rval, StringValue);
   #undef GETSTAT
#endif // MO_DYNO_COUNTS

#ifdef MO_DYNO_KEY_COUNTS
   {
      _stats_key_counts_lock.lock();
      DynamicObject& d = rval["KeyCounts"];
      d["count"] = _stats_key_counts.size();
      d["keys"]->setType(Map);
      _StatsKeyMap::iterator i =
         _stats_key_counts.begin();
      for(; i != _stats_key_counts.end(); ++i)
      {
         // create new stat entry
         DynamicObject& s = d["keys"][i->first];
         MAKESTAT(s, _stats_key_counts[i->first]);
      }
      _stats_key_counts_lock.unlock();
   }
#endif // MO_DYNO_KEY_COUNTS

   return rval;
}
