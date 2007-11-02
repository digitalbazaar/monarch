/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_util_DynamicObjectImpl_H
#define db_util_DynamicObjectImpl_H

#include <map>
#include <vector>

namespace db
{
namespace util
{

// forward declare DynamicObject
class DynamicObject;

/**
 * A DynamicObjectImpl is an implementation for a DynamicObject.
 * 
 * @author Dave Longley
 */
class DynamicObjectImpl
{
public:
   /**
    * The possible types for this object.
    */
   typedef enum Type
   {
      String, Boolean, Int32, UInt32, Int64, UInt64, Map, Array
   };
   
protected:
   /**
    * A MemberComparator compares two member names.
    */
   typedef struct MemberComparator
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
   
   /**
    * The type for this object.
    */
   Type mType;
   
   /**
    * The value for this object.
    */
   union
   {
      char* mString;
      bool mBoolean;
      int mInt32;
      unsigned int mUInt32;
      long long mInt64;
      unsigned long long mUInt64;
      ObjectMap* mMap;
      ObjectArray* mArray;
   };
   
   /**
    * Frees the data associated with this DynamicObjectImpl.
    */
   virtual void freeData();
   
   /**
    * A helper function that sets this object's value to the passed string.
    * 
    * @param value the string value for this object.
    */
   virtual void setString(const char* value);
   
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
    * Sets this object's value to a string.
    * 
    * @param value the value for this object.
    */
   virtual void operator=(const std::string& value);
   
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
   virtual void operator=(int value);
   
   /**
    * Sets this object's value to a 32-bit unsigned integer.
    * 
    * @param value the value for this object.
    */
   virtual void operator=(unsigned int value);
   
   /**
    * Sets this object's value to a 64-bit integer.
    * 
    * @param value the value for this object.
    */
   virtual void operator=(long long value);
   
   /**
    * Sets this object's value to a 64-bit unsigned integer.
    * 
    * @param value the value for this object.
    */
   virtual void operator=(unsigned long long value);
   
   /**
    * Gets a DynamicObject based on its member name.
    * 
    * @param name the name of the member.
    * 
    * @return the DynamicObject.
    */
   virtual DynamicObject& operator[](const std::string& name);
   
   /**
    * Gets a DynamicObject based on its index.
    * 
    * @param index the index of the member.
    * 
    * @return the DynamicObject.
    */
   virtual DynamicObject& operator[](unsigned int index);
   
   /**
    * Gets this object's type.
    * 
    * @return the type of this object.
    */
   virtual Type getType();
   
   /**
    * Gets this object's value as a string.
    * 
    * @return the value of this object.
    */
   virtual const char* getString();
   
   /**
    * Gets this object's value as a boolean.
    * 
    * @return the value of this object.
    */
   virtual bool getBoolean();
   
   /**
    * Gets this object's value as a 32-bit integer.
    * 
    * @return the value of this object.
    */
   virtual int getInt32();
   
   /**
    * Gets this object's value as a 32-bit unsigned integer.
    * 
    * @return the value of this object.
    */
   virtual int getUInt32();
   
   /**
    * Gets this object's value as a 64-bit integer.
    * 
    * @return the value of this object.
    */
   virtual int getInt64();
   
   /**
    * Gets this object's value as a 64-bit unsigned integer.
    * 
    * @return the value of this object.
    */
   virtual int getUInt64();
   
   /**
    * Returns true if this object has the specified member, false if not.
    * 
    * @param name the name of the member.
    * 
    * @return true if this object has a member with the specified name,
    *         false if not.
    */
   virtual bool hasMember(const char* name);
   
   /**
    * Gets the length of this object based on its type.
    * 
    * String length = number of characters in the string.
    * Number length = size of integer type.
    * Boolean length = 1
    * Map length = number of entries in the map.
    * Array length = number of elements in the array.
    */
   virtual unsigned int length();
};

} // end namespace util
} // end namespace db
#endif
