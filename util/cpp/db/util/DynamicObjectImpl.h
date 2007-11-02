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
    * Types for a MemberValue.
    */
   typedef enum MemberValueType
   {
      String, Boolean, Int32, UInt32, Int64, UInt64, Object, Array
   };
   
   /**
    * A MemberValue. A MemberValue may consist of a string, a DynamicObjectImpl,
    * or an array of other MemberValues.
    */
   typedef struct MemberValue
   {
      /**
       * The type for this value. 
       */
      MemberValueType type;
      
      /**
       * The data for this value.
       */
      union
      {
         // Note: numbers and boolean values are internally converted to
         // strings, but since this is not space efficient, it may change
         // internally in the future
         char* str;
         DynamicObject* obj;
         std::vector<MemberValue>* array;
      };
      
      /**
       * Creates an initialized MemberValue.
       */
      MemberValue();
      
      /**
       * Frees the data for a MemberValue.
       * 
       * @param mv the MemberValue to update.
       */
      void freeData();
      
      // operators for setting member value data
      void operator=(const char* rhs);
      void operator=(bool rhs);
      void operator=(int rhs);
      void operator=(unsigned int rhs);
      void operator=(long long rhs);
      void operator=(unsigned long long rhs);
      void operator=(DynamicObject rhs);
      
      // operators for array access
      MemberValue& operator[](unsigned int index);
   };
   
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
   
protected:
   /**
    * This DynamicObjectImpl's members.
    */
   typedef std::map<const char*, MemberValue, MemberComparator> MemberMap;
   MemberMap mMembers;
   
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
    * Gets a MemberValue based on its member name.
    * 
    * @param name the name of the member.
    * 
    * @return the MemberValue.
    */
   virtual MemberValue& operator[](const char* name);
   
   /**
    * Sets a string member in this DynamicObjectImpl.
    * 
    * @param name the name of the member.
    * @param value the value of the member.
    */
   virtual void setMember(const char* name, const char* value);
   
   /**
    * Sets a boolean member in this DynamicObjectImpl.
    * 
    * @param name the name of the member.
    * @param value the value of the member.
    */
   virtual void setMember(const char* name, bool value);
   
   /**
    * Sets a 32-bit integer member in this DynamicObjectImpl.
    * 
    * @param name the name of the member.
    * @param value the value of the member.
    */
   virtual void setMember(const char* name, int value);
   
   /**
    * Sets a 32-bit unsigned integer member in this DynamicObjectImpl.
    * 
    * @param name the name of the member.
    * @param value the value of the member.
    */
   virtual void setMember(const char* name, unsigned int value);
   
   /**
    * Sets a 64-bit integer member in this DynamicObjectImpl.
    * 
    * @param name the name of the member.
    * @param value the value of the member.
    */
   virtual void setMember(const char* name, long long value);
   
   /**
    * Sets a 64-bit unsigned integer member in this DynamicObjectImpl.
    * 
    * @param name the name of the member.
    * @param value the value of the member.
    */
   virtual void setMember(const char* name, unsigned long long value);
   
   /**
    * Sets a DynamicObject member in this DynamicObjectImpl.
    * 
    * @param name the name of the member.
    * @param value the value of the member.
    */
   virtual void setMember(const char* name, DynamicObject value);
   
   /**
    * Gets a string member from this DynamicObjectImpl.
    * 
    * @param name the name of the member.
    * 
    * @return the value of the member.
    */
   virtual const char* getString(const char* name);
   
   /**
    * Gets a boolean member from this DynamicObjectImpl.
    * 
    * @param name the name of the member.
    * 
    * @return the value of the member.
    */
   virtual bool getBoolean(const char* name);
   
   /**
    * Gets a 32-bit integer member from this DynamicObjectImpl.
    * 
    * @param name the name of the member.
    * 
    * @return the value of the member.
    */
   virtual int getInt32(const char* name);
   
   /**
    * Gets a 32-bit unsigned integer member from this DynamicObjectImpl.
    * 
    * @param name the name of the member.
    * 
    * @return the value of the member.
    */
   virtual unsigned int getUInt32(const char* name);
   
   /**
    * Gets a 64-bit integer member from this DynamicObjectImpl.
    * 
    * @param name the name of the member.
    * 
    * @return the value of the member.
    */
   virtual long long getInt64(const char* name);
   
   /**
    * Gets a 64-bit unsigned integer member from this DynamicObjectImpl.
    * 
    * @param name the name of the member.
    * 
    * @return value the value of the member.
    */
   virtual unsigned long long getUInt64(const char* name);
   
   /**
    * Gets a DynamicObject member from this DynamicObjectImpl.
    * 
    * @param name the name of the member.
    * 
    * @return value the value of the member.
    */
   virtual DynamicObject getObject(const char* name);
   
   /**
    * Gets an array member from this DynamicObjectImpl.
    * 
    * @param name the name of the member.
    * 
    * @return the value of the member.
    */
   std::vector<DynamicObjectImpl::MemberValue>& getArray(const char* name);
};

} // end namespace util
} // end namespace db
#endif
