/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_rt_DynamicObjectImpl_H
#define monarch_rt_DynamicObjectImpl_H

#include <map>
#include <vector>
#include <cstring>
#include <cstdarg>

#include <inttypes.h>

namespace monarch
{
namespace rt
{

// forward declare DynamicObject, DynamicObjectIteratorImpl
class DynamicObject;
class DynamicObjectIteratorImpl;

/**
 * The possible types for a DynamicObject.
 */
enum DynamicObjectType
{
   String = 0, Boolean, Int32, UInt32, Int64, UInt64, Double, Map, Array,
   LastDynamicObjectType = Array
};

/**
 * A DynamicObjectImpl is an implementation for a DynamicObject.
 *
 * @author Dave Longley
 */
class DynamicObjectImpl
{
public:
   /**
    * A MemberComparator compares two member names.
    */
   struct MemberComparator
   {
      /**
       * Compares two null-terminated strings, returning true if the first is
       * less than the second, false if not. The compare is case-sensitive.
       *
       * @param s1 the first string.
       * @param s2 the second string.
       *
       * @return true if the s1 < s2, false if not.
       */
      bool operator()(const char* s1, const char* s2) const
      {
         return strcmp(s1, s2) < 0;
      }
   };

   /**
    * The definition for a DynamicObject map and array.
    */
   typedef std::map<const char*, DynamicObject, MemberComparator> ObjectMap;
   typedef std::vector<DynamicObject> ObjectArray;

protected:
   /**
    * The type for this object.
    */
   DynamicObjectType mType;

   /**
    * The value for this object.
    */
   union
   {
      char* mString;
      bool mBoolean;
      int32_t mInt32;
      uint32_t mUInt32;
      int64_t mInt64;
      uint64_t mUInt64;
      double mDouble;
      ObjectMap* mMap;
      ObjectArray* mArray;
   };

   /**
    * The value for this object as a string. This is used when the string
    * version of this object is requested when its type is not a string.
    */
   char* mStringValue;

   /**
    * Allow access to iterators.
    */
   friend class DynamicObject;
   friend class DynamicObjectIteratorArray;
   friend class DynamicObjectIteratorMap;

public:
   /**
    * Creates a new DynamicObjectImpl.
    */
   DynamicObjectImpl();

   /**
    * Destructs this DynamicObjectImpl.
    */
   virtual ~DynamicObjectImpl();

   /**
    * Sets this object's value to the value of another DynamicObjectImpl.
    *
    * @param value the value for this object.
    */
   virtual void operator=(const DynamicObjectImpl& value);

   /**
    * Sets this object's value to a string.
    *
    * @param value the value for this object.
    */
   virtual void operator=(const char* value);

   /**
    * Sets this object's value to a boolean.
    *
    * @param value the value for this object.
    */
   virtual void operator=(bool value);

   /**
    * Sets this object's value to a 32-bit integer.
    *
    * @param value the value for this object.
    */
   virtual void operator=(int32_t value);

   /**
    * Sets this object's value to a 32-bit unsigned integer.
    *
    * @param value the value for this object.
    */
   virtual void operator=(uint32_t value);

   /**
    * Sets this object's value to a 64-bit integer.
    *
    * @param value the value for this object.
    */
   virtual void operator=(int64_t value);

   /**
    * Sets this object's value to a 64-bit unsigned integer.
    *
    * @param value the value for this object.
    */
   virtual void operator=(uint64_t value);

   /**
    * Sets this object's value to a double.
    *
    * @param value the value for this object.
    */
   virtual void operator=(double value);

   /**
    * Gets a DynamicObject based on its member name.
    *
    * @param name the name of the member.
    *
    * @return the DynamicObject.
    */
   virtual DynamicObject& operator[](const char* name);

   /**
    * Gets a DynamicObject based on its index. A negative index will
    * index in reverse, with -1 referring to the last element.
    *
    * @param index the index of the member.
    *
    * @return the DynamicObject.
    */
   virtual DynamicObject& operator[](int index);

   /**
    * Compares this object to another one for equality. If the objects are
    * the same type, and have the same value or contain all the same values
    * (for Maps/Arrays), then they are equal.
    *
    * @param rhs the DynamicObjectImpl to compare to this one.
    *
    * @return true if the passed object is equal to this one, false if not.
    */
   virtual bool operator==(const DynamicObjectImpl& rhs) const;

   /**
    * Compares this object's value to another object's value to see if this
    * one is less.
    *
    * If one object is NULL, it is less.
    *
    * If the objects are both numbers, their operator< will be used.
    *
    * If they are both booleans, false will be less than true.
    *
    * If they are both strings, strcmp will be used.
    *
    * If they are both maps, then the smaller map will be less. If the maps are
    * the same size, then their keys will be compared lexigraphically using
    * operator<. If their keys are the same, the values of their keys will be
    * compared using operator<.
    *
    * If they are both arrays, then operator< will be used.
    *
    * If they are different types, then their string value will be compared.
    *
    * @param rhs the other object.
    *
    * @return true if this object is less, false if not.
    */
   virtual bool operator<(const DynamicObjectImpl& rhs) const;

   /**
    * Appends a DynamicObject to this one and returns a reference to it for
    * it to be set to a value.
    *
    * @return the DynamicObject to be set.
    */
   virtual DynamicObject& append();

   /**
    * Appends the passed DynamicObject to this one and returns a reference to
    * it.
    *
    * @param value the DynamicObject to append.
    *
    * @return the DynamicObject that was appended.
    */
   virtual DynamicObject& append(DynamicObject& value);

   /**
    * Creates a DynamicObject, appends it to this one, and returns a reference
    * to it.
    *
    * @param value the value to append.
    *
    * @return the DynamicObject that was appended.
    */
   virtual DynamicObject& append(const char* value);

   /**
    * Creates a DynamicObject, appends it to this one, and returns a reference
    * to it.
    *
    * @param value the value to append.
    *
    * @return the DynamicObject that was appended.
    */
   virtual DynamicObject& append(bool value);

   /**
    * Creates a DynamicObject, appends it to this one, and returns a reference
    * to it.
    *
    * @param value the value to append.
    *
    * @return the DynamicObject that was appended.
    */
   virtual DynamicObject& append(int32_t value);

   /**
    * Creates a DynamicObject, appends it to this one, and returns a reference
    * to it.
    *
    * @param value the value to append.
    *
    * @return the DynamicObject that was appended.
    */
   virtual DynamicObject& append(uint32_t value);

   /**
    * Creates a DynamicObject, appends it to this one, and returns a reference
    * to it.
    *
    * @param value the value to append.
    *
    * @return the DynamicObject that was appended.
    */
   virtual DynamicObject& append(int64_t value);

   /**
    * Creates a DynamicObject, appends it to this one, and returns a reference
    * to it.
    *
    * @param value the value to append.
    *
    * @return the DynamicObject that was appended.
    */
   virtual DynamicObject& append(uint64_t value);

   /**
    * Creates a DynamicObject, appends it to this one, and returns a reference
    * to it.
    *
    * @param value the value to append.
    *
    * @return the DynamicObject that was appended.
    */
   virtual DynamicObject& append(double value);

   /**
    * Sets this object's type. Its existing value will be converted if
    * possible.
    *
    * @param type the type to set this object to.
    */
   virtual void setType(DynamicObjectType type);

   /**
    * Gets this object's type.
    *
    * @return the type of this object.
    */
   virtual DynamicObjectType getType() const;

   /**
    * Sets this object to a printf-formatted string.
    *
    * @param format the format for the string.
    * @param ... the format parameters.
    */
   virtual void format(const char* format, ...)
#ifdef __GNUC__
      __attribute__ ((format (printf, 2, 3)))
#endif
         ;

   /**
    * Gets this object's value as a string. If the type of this object
    * is not a string, then the returned pointer may be invalidated by
    * the next call to getString().
    *
    * @return the value of this object.
    */
   virtual const char* getString() const;

   /**
    * Gets this object's value as a boolean.
    *
    * @return the value of this object.
    */
   virtual bool getBoolean() const;

   /**
    * Gets this object's value as a 32-bit integer.
    *
    * @return the value of this object.
    */
   virtual int32_t getInt32() const;

   /**
    * Gets this object's value as a 32-bit unsigned integer.
    *
    * @return the value of this object.
    */
   virtual uint32_t getUInt32() const;

   /**
    * Gets this object's value as a 64-bit integer.
    *
    * @return the value of this object.
    */
   virtual int64_t getInt64() const;

   /**
    * Gets this object's value as a 64-bit unsigned integer.
    *
    * @return the value of this object.
    */
   virtual uint64_t getUInt64() const;

   /**
    * Gets this object's value as a double.
    *
    * @return the value of this object.
    */
   virtual double getDouble() const;

   /**
    * Returns true if this object has the specified member, false if not.
    *
    * @param name the name of the member.
    *
    * @return true if this object has a member with the specified name,
    *         false if not.
    */
   virtual bool hasMember(const char* name) const;

   /**
    * Removes a member from this object, if it has it.
    *
    * @param name the member to remove from this object.
    */
   virtual void removeMember(const char* name);

   /**
    * If this object is an Array and at least one of its indexes has an
    * object that is equal to the passed object, this method returns the
    * index of that object. Otherwise, it returns -1.
    *
    * @param obj the object to look for.
    *
    * @return if this object is an Array and contains the passed object,
    *         the index of that object, otherwise -1.
    */
   virtual int indexOf(DynamicObject& obj) const;

   /**
    * If this object is an Array and at least one of its indexes has an
    * object that is equal to the passed object, this method returns the
    * index of that object. Otherwise, it returns -1.
    *
    * @param obj the object to look for.
    *
    * @return if this object is an Array and contains the passed object,
    *         the index of that object, otherwise -1.
    */
   virtual int indexOf(const char* value) const;

   /**
    * If this object is an Array and at least one of its indexes has an
    * object that is equal to the passed object, this method returns the
    * index of that object. Otherwise, it returns -1.
    *
    * @param obj the object to look for.
    *
    * @return if this object is an Array and contains the passed object,
    *         the index of that object, otherwise -1.
    */
   virtual int indexOf(bool value) const;

   /**
    * If this object is an Array and at least one of its indexes has an
    * object that is equal to the passed object, this method returns the
    * index of that object. Otherwise, it returns -1.
    *
    * @param obj the object to look for.
    *
    * @return if this object is an Array and contains the passed object,
    *         the index of that object, otherwise -1.
    */
   virtual int indexOf(int32_t value) const;

   /**
    * If this object is an Array and at least one of its indexes has an
    * object that is equal to the passed object, this method returns the
    * index of that object. Otherwise, it returns -1.
    *
    * @param obj the object to look for.
    *
    * @return if this object is an Array and contains the passed object,
    *         the index of that object, otherwise -1.
    */
   virtual int indexOf(uint32_t value) const;

   /**
    * If this object is an Array and at least one of its indexes has an
    * object that is equal to the passed object, this method returns the
    * index of that object. Otherwise, it returns -1.
    *
    * @param obj the object to look for.
    *
    * @return if this object is an Array and contains the passed object,
    *         the index of that object, otherwise -1.
    */
   virtual int indexOf(int64_t value) const;

   /**
    * If this object is an Array and at least one of its indexes has an
    * object that is equal to the passed object, this method returns the
    * index of that object. Otherwise, it returns -1.
    *
    * @param obj the object to look for.
    *
    * @return if this object is an Array and contains the passed object,
    *         the index of that object, otherwise -1.
    */
   virtual int indexOf(uint64_t value) const;

   /**
    * If this object is an Array and at least one of its indexes has an
    * object that is equal to the passed object, this method returns the
    * index of that object. Otherwise, it returns -1.
    *
    * @param obj the object to look for.
    *
    * @return if this object is an Array and contains the passed object,
    *         the index of that object, otherwise -1.
    */
   virtual int indexOf(double value) const;

   /**
    * Clears associated data.
    * Maps and Arrays: removes all items.
    * Strings: set to "".
    * Numbers: set to 0.
    */
   virtual void clear();

   /**
    * Gets the length of this object based on its type.
    *
    * String length = number of characters in the string.
    * Number length = size of integer type.
    * Boolean length = 1
    * Map length = number of entries in the map.
    * Array length = number of elements in the array.
    *
    * @return the length of this object or -1 if unknown
    */
   virtual int length() const;

   /**
    * Reverse the data.
    * Arrays: reverse.
    * Strings: reverse.
    * Other: do nothing.
    */
   virtual void reverse();

   /**
    * Enable or disable debugging statistics.
    *
    * Note: Must be enabled with a compile time option.
    *
    * @param enable true to enable, false to disable.
    *
    * @return the previous enabled value
    */
   static bool enableStats(bool enable);

   /**
    * Clear debugging statistics.
    *
    * Note: Must be enabled with a compile time option.
    */
   static void clearStats();

   /**
    * Return a structure with DynamicObject debugging statistics.
    *
    * Note: Must be enabled with a compile time option.
    *
    * @return map of statistics if enabled else an empty map.
    */
   static DynamicObject getStats();

protected:
   /**
    * Frees the key data associated with a Map.
    */
   virtual void freeMapKeys();

   /**
    * Frees the data associated with this DynamicObjectImpl.
    */
   virtual void freeData();

   /**
    * Removes a member from this map object.
    *
    * @param it iterator to the member to remove from this object.
    */
   virtual void removeMember(ObjectMap::iterator iterator);

   /**
    * Sets this object to the passed formatted string.
    *
    * @param format the format for the url.
    * @param varargs the format parameters.
    */
   virtual void setFormattedString(const char* format, va_list varargs);
};

} // end namespace rt
} // end namespace monarch
#endif
