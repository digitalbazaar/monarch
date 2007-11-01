/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_data_DataMappingFunctor_H
#define db_data_DataMappingFunctor_H

#include "db/data/DataMapping.h"
#include "db/io/ByteBuffer.h"

namespace db
{
namespace data
{

/**
 * A DataMappingFunctor allows a specific object to have its data binding
 * methods associated with a universal name (namespace + name). This class
 * can be used to set data in a bound object or retrieve data from a bound
 * object. It can also be used to create another object and add it to a
 * bound object.
 * 
 * @author Dave Longley
 */
template<class BoundType, class ChildType = void>
class DataMappingFunctor : public DataMapping
{
protected:
   /**
    * Typedef for creating a child object.
    */
   typedef ChildType* (BoundType::*CreateChildFunction)();
   
   /**
    * Typedef for adding a child object.
    */
   typedef void (BoundType::*AddChildFunction)(ChildType*);
   
   /**
    * Typedefs for setting data in an object.
    */
   typedef void (BoundType::*SetBooleanFunction)(bool);
   typedef void (BoundType::*SetInt32Function)(int);
   typedef void (BoundType::*SetUInt32Function)(unsigned int);
   typedef void (BoundType::*SetInt64Function)(long long);
   typedef void (BoundType::*SetUInt64Function)(unsigned long long);
   typedef void (BoundType::*SetStringFunction)(const char*);
   
   /**
    * Typedefs for getting data in an object.
    */
   typedef bool (BoundType::*GetBooleanFunction)();
   typedef int (BoundType::*GetInt32Function)();
   typedef unsigned int (BoundType::*GetUInt32Function)();
   typedef long long (BoundType::*GetInt64Function)();
   typedef unsigned long long (BoundType::*GetUInt64Function)();
   typedef const char* (BoundType::*GetStringFunction)();
   typedef bool (BoundType::*GetBooleanConstFunction)() const;
   typedef int (BoundType::*GetInt32ConstFunction)() const;
   typedef unsigned int (BoundType::*GetUInt32ConstFunction)() const;
   typedef long long (BoundType::*GetInt64ConstFunction)() const;
   typedef unsigned long long (BoundType::*GetUInt64ConstFunction)() const;
   typedef const char* (BoundType::*GetStringConstFunction)() const;
   
   /**
    * A function for setting data in an object.
    */
   typedef struct DataSetFunction
   {
      /**
       * The type of data the function sets.
       */
      enum Type
      {
         None, Boolean, Int32, UInt32, Int64, UInt64, String
      } type;
      
      /**
       * The pointer to the set function.
       */
      union
      {
         SetBooleanFunction bFunc;
         SetInt32Function i32Func;
         SetUInt32Function ui32Func;
         SetInt64Function i64Func;
         SetUInt64Function ui64Func;
         SetStringFunction sFunc;
      };
   };
   
   /**
    * A function for getting data from an object.
    */
   typedef struct DataGetFunction
   {
      /**
       * The type of data the function gets.
       */
      enum Type
      {
         None, Boolean, Int32, UInt32, Int64, UInt64, String,
         BooleanConst, Int32Const, UInt32Const, Int64Const, UInt64Const,
         StringConst
      } type;
      
      /**
       * The pointer to the get function.
       */
      union
      {
         // non-const functions
         GetBooleanFunction bFunc;
         GetInt32Function i32Func;
         GetUInt32Function ui32Func;
         GetInt64Function i64Func;
         GetUInt64Function ui64Func;
         GetStringFunction sFunc;
         
         // const functions
         GetBooleanConstFunction bcFunc;
         GetInt32ConstFunction i32cFunc;
         GetUInt32ConstFunction ui32cFunc;
         GetInt64ConstFunction i64cFunc;
         GetUInt64ConstFunction ui64cFunc;
         GetStringConstFunction scFunc;
      };
   };
   
   /**
    * A function for creating a child object.
    */
   CreateChildFunction mCreateFunction;
   
   /**
    * A function for adding a child object.
    */
   AddChildFunction mAddFunction;
   
   /**
    * The DataSetFunction for setting data in the object.
    */
   DataSetFunction mSetFunction;
   
   /**
    * The DataGetFunction for getting data from the object.
    */
   DataGetFunction mGetFunction;
   
   /**
    * A ByteBuffer for storing temporary data.
    */
   db::io::ByteBuffer mDataCache;
   
public:
   /**
    * Creates a new DataMappingFunctor.
    * 
    * @param cFunc a function for creating a child object.
    * @param aFunc a function for adding a child object to the bound object.
    */
   DataMappingFunctor(
      CreateChildFunction cFunc, AddChildFunction aFunc);
   
   /**
    * Creates a new DataMappingFunctor.
    * 
    * @param sbFunc a function for setting data in the bound object.
    * @param gbFunc a function for getting data from the bound object.
    */
   DataMappingFunctor(
      SetBooleanFunction sbFunc, GetBooleanFunction gbFunc);
   
   /**
    * Creates a new DataMappingFunctor.
    * 
    * @param siFunc a function for setting data in the bound object.
    * @param giFunc a function for getting data from the bound object.
    */
   DataMappingFunctor(
      SetInt32Function siFunc, GetInt32Function giFunc);
   
   /**
    * Creates a new DataMappingFunctor.
    * 
    * @param siFunc a function for setting data in the bound object.
    * @param giFunc a function for getting data from the bound object.
    */
   DataMappingFunctor(
      SetUInt32Function siFunc, GetUInt32Function giFunc);
   
   /**
    * Creates a new DataMappingFunctor.
    * 
    * @param siFunc a function for setting data in the bound object.
    * @param giFunc a function for getting data from the bound object.
    */
   DataMappingFunctor(
      SetInt64Function siFunc, GetInt64Function giFunc);
   
   /**
    * Creates a new DataMappingFunctor.
    * 
    * @param siFunc a function for setting data in the bound object.
    * @param giFunc a function for getting data from the bound object.
    */
   DataMappingFunctor(
      SetUInt64Function siFunc, GetUInt64Function giFunc);
   
   /**
    * Creates a new DataMappingFunctor.
    * 
    * @param ssFunc a function for setting data in an object.
    * @param gsFunc a function for getting data from an object.
    */
   DataMappingFunctor(
      SetStringFunction ssFunc, GetStringFunction gsFunc);
   
   /**
    * Creates a new DataMappingFunctor.
    * 
    * @param sbFunc a function for setting data in the bound object.
    * @param gbFunc a function for getting data from the bound object.
    */
   DataMappingFunctor(
      SetBooleanFunction sbFunc, GetBooleanConstFunction gbFunc);
   
   /**
    * Creates a new DataMappingFunctor.
    * 
    * @param siFunc a function for setting data in the bound object.
    * @param giFunc a function for getting data from the bound object.
    */
   DataMappingFunctor(
      SetInt32Function siFunc, GetInt32ConstFunction giFunc);
   
   /**
    * Creates a new DataMappingFunctor.
    * 
    * @param siFunc a function for setting data in the bound object.
    * @param giFunc a function for getting data from the bound object.
    */
   DataMappingFunctor(
      SetUInt32Function siFunc, GetUInt32ConstFunction giFunc);
   
   /**
    * Creates a new DataMappingFunctor.
    * 
    * @param siFunc a function for setting data in the bound object.
    * @param giFunc a function for getting data from the bound object.
    */
   DataMappingFunctor(
      SetInt64Function siFunc, GetInt64ConstFunction giFunc);
   
   /**
    * Creates a new DataMappingFunctor.
    * 
    * @param siFunc a function for setting data in the bound object.
    * @param giFunc a function for getting data from the bound object.
    */
   DataMappingFunctor(
      SetUInt64Function siFunc, GetUInt64ConstFunction giFunc);
   
   /**
    * Creates a new DataMappingFunctor.
    * 
    * @param ssFunc a function for setting data in an object.
    * @param gsFunc a function for getting data from an object.
    */
   DataMappingFunctor(
      SetStringFunction ssFunc, GetStringConstFunction gsFunc);
   
   /**
    * Destructs this DataMappingFunctor.
    */
   virtual ~DataMappingFunctor();
   
   /**
    * Creates a child object to add to the bound object.
    * 
    * @param bObject the bound object.
    * 
    * @return a pointer to the child object.
    */
   virtual void* createChild(void* bObject);
   
   /**
    * Adds a child object to the bound object.
    * 
    * @param bObject the bound object.
    * @param cObject the child object to add to the bound object.
    */
   virtual void addChild(void* bObject, void* cObject);   
   
   /**
    * Sets the passed data in the bound object.
    * 
    * @param bObject the bound object.
    * @param data the data to set in the object.
    * @param length the length of the data.
    */
   virtual void setData(void* bObject, const char* data, int length);
   
   /**
    * Sets the passed data in the bound object by interpreting the passed
    * pointer as a pointer to the raw memory for the data. For instance,
    * if the data type is a 32-bit integer, the data pointer will point at
    * 4 bytes.
    * 
    * @param bObject the bound object.
    * @param data the data to set in the object.
    * @param length the length of the data.
    */
   virtual void setRawData(void* bObject, char* data, int length);
   
   /**
    * Appends the passed data to the bound object.
    * 
    * @param bObject the bound object.
    * @param data the data to set in the object.
    * @param length the length of the data.
    */
   virtual void appendData(void* bObject, const char* data, int length);
   
   /**
    * Ends data being set to the bound object. This method should flush any
    * cached data (via setData() or appendData()) to the bound object.
    * 
    * @param bObject the bound object.
    */
   virtual void endData(void* bObject);
   
   /**
    * Gets data from the bound object.
    * 
    * The caller of this method is responsible for freeing the returned
    * data.
    * 
    * @param bObject the bound object.
    * @param s a pointer to point at the data (null-terminated) from the bound
    *          object.
    */
   virtual void getData(void* bObject, char** s);
   
   /**
    * Gets raw data from the bound object. No conversion will be performed
    * on the data. For instance, if the data is an 32-bit integer, then a
    * buffer of 4 bytes will be allocated and "s" will be pointed at it.
    * 
    * The caller of this method is responsible for freeing the returned
    * data.
    * 
    * @param bObject the bound object.
    * @param s a pointer to point at the data from the bound object.
    */
   virtual void getRawData(void* bObject, char** s);
   
   /**
    * Writes the data for the passed bound object to the given output stream.
    * 
    * @param bObject the bound object.
    * @param os the OutputStream to write to.
    */
   virtual bool writeData(void* bObject, db::io::OutputStream* os);
   
   /**
    * Returns true if the passed bound object has data, false if not.
    * 
    * @param bObject the bound object to check for data.
    * 
    * @return true if the passed bound object has data, false if not.
    */
   virtual bool hasData(void* bObject);
   
   /**
    * Returns the type of data for this mapping.
    * 
    * @return the type of data for this mapping.
    */
   virtual DataType getDataType();
   
   /**
    * True if this DataMapping is a create/add child mapping, false if it is a
    * set/get mapping. 
    * 
    * @return true if this DataMapping is a create/add child mapping, false if
    *         it is a set/get mapping.
    */
   virtual bool isChildMapping();
};

template<class BoundType, class ChildType>
DataMappingFunctor<BoundType, ChildType>::DataMappingFunctor(
   CreateChildFunction cFunc, AddChildFunction aFunc)
{
   // set create/add functions
   mCreateFunction = cFunc;
   mAddFunction = aFunc;
   
   // no set/get functions
   mSetFunction.type = DataSetFunction::None;
   mGetFunction.type = DataGetFunction::None;
}

template<class BoundType, class ChildType>
DataMappingFunctor<BoundType, ChildType>::DataMappingFunctor(
   SetBooleanFunction sbFunc, GetBooleanFunction gbFunc)
{
   // no create/add functions
   mCreateFunction = NULL;
   mAddFunction = NULL;
   
   // set set/get functions
   mSetFunction.type = DataSetFunction::Boolean;
   mSetFunction.bFunc = sbFunc;
   mGetFunction.type = DataGetFunction::Boolean;
   mGetFunction.bFunc = gbFunc;
}

template<class BoundType, class ChildType>
DataMappingFunctor<BoundType, ChildType>::DataMappingFunctor(
   SetInt32Function siFunc, GetInt32Function giFunc)
{
   // no create/add functions
   mCreateFunction = NULL;
   mAddFunction = NULL;
   
   // set set/get functions
   mSetFunction.type = DataSetFunction::Int32;
   mSetFunction.i32Func = siFunc;
   mGetFunction.type = DataGetFunction::Int32;
   mGetFunction.i32Func = giFunc;
}

template<class BoundType, class ChildType>
DataMappingFunctor<BoundType, ChildType>::DataMappingFunctor(
   SetUInt32Function siFunc, GetUInt32Function giFunc)
{
   // no create/add functions
   mCreateFunction = NULL;
   mAddFunction = NULL;
   
   // set set/get functions
   mSetFunction.type = DataSetFunction::UInt32;
   mSetFunction.ui32Func = siFunc;
   mGetFunction.type = DataGetFunction::UInt32;
   mGetFunction.ui32Func = giFunc;
}

template<class BoundType, class ChildType>
DataMappingFunctor<BoundType, ChildType>::DataMappingFunctor(
   SetInt64Function siFunc, GetInt64Function giFunc)
{
   // no create/add functions
   mCreateFunction = NULL;
   mAddFunction = NULL;
   
   // set set/get functions
   mSetFunction.type = DataSetFunction::Int64;
   mSetFunction.i64Func = siFunc;
   mGetFunction.type = DataGetFunction::Int64;
   mGetFunction.i64Func = giFunc;
}

template<class BoundType, class ChildType>
DataMappingFunctor<BoundType, ChildType>::DataMappingFunctor(
   SetUInt64Function siFunc, GetUInt64Function giFunc)
{
   // no create/add functions
   mCreateFunction = NULL;
   mAddFunction = NULL;
   
   // set set/get functions
   mSetFunction.type = DataSetFunction::UInt64;
   mSetFunction.ui64Func = siFunc;
   mGetFunction.type = DataGetFunction::UInt64;
   mGetFunction.ui64Func = giFunc;
}

template<class BoundType, class ChildType>
DataMappingFunctor<BoundType, ChildType>::DataMappingFunctor(
   SetStringFunction ssFunc, GetStringFunction gsFunc)
{
   // no create/add functions
   mCreateFunction = NULL;
   mAddFunction = NULL;
   
   // set set/get functions
   mSetFunction.type = DataSetFunction::String;
   mSetFunction.sFunc = ssFunc;
   mGetFunction.type = DataGetFunction::String;
   mGetFunction.sFunc = gsFunc;
}

template<class BoundType, class ChildType>
DataMappingFunctor<BoundType, ChildType>::DataMappingFunctor(
   SetBooleanFunction sbFunc, GetBooleanConstFunction gbFunc)
{
   // no create/add functions
   mCreateFunction = NULL;
   mAddFunction = NULL;
   
   // set set/get functions
   mSetFunction.type = DataSetFunction::Boolean;
   mSetFunction.bFunc = sbFunc;
   mGetFunction.type = DataGetFunction::BooleanConst;
   mGetFunction.bcFunc = gbFunc;
}

template<class BoundType, class ChildType>
DataMappingFunctor<BoundType, ChildType>::DataMappingFunctor(
   SetInt32Function siFunc, GetInt32ConstFunction giFunc)
{
   // no create/add functions
   mCreateFunction = NULL;
   mAddFunction = NULL;
   
   // set set/get functions
   mSetFunction.type = DataSetFunction::Int32;
   mSetFunction.i32Func = siFunc;
   mGetFunction.type = DataGetFunction::Int32Const;
   mGetFunction.i32cFunc = giFunc;
}

template<class BoundType, class ChildType>
DataMappingFunctor<BoundType, ChildType>::DataMappingFunctor(
   SetUInt32Function siFunc, GetUInt32ConstFunction giFunc)
{
   // no create/add functions
   mCreateFunction = NULL;
   mAddFunction = NULL;
   
   // set set/get functions
   mSetFunction.type = DataSetFunction::UInt32;
   mSetFunction.ui32Func = siFunc;
   mGetFunction.type = DataGetFunction::UInt32Const;
   mGetFunction.ui32cFunc = giFunc;
}

template<class BoundType, class ChildType>
DataMappingFunctor<BoundType, ChildType>::DataMappingFunctor(
   SetInt64Function siFunc, GetInt64ConstFunction giFunc)
{
   // no create/add functions
   mCreateFunction = NULL;
   mAddFunction = NULL;
   
   // set set/get functions
   mSetFunction.type = DataSetFunction::Int64;
   mSetFunction.i64Func = siFunc;
   mGetFunction.type = DataGetFunction::Int64Const;
   mGetFunction.i64cFunc = giFunc;
}

template<class BoundType, class ChildType>
DataMappingFunctor<BoundType, ChildType>::DataMappingFunctor(
   SetUInt64Function siFunc, GetUInt64ConstFunction giFunc)
{
   // no create/add functions
   mCreateFunction = NULL;
   mAddFunction = NULL;
   
   // set set/get functions
   mSetFunction.type = DataSetFunction::UInt64;
   mSetFunction.ui64Func = siFunc;
   mGetFunction.type = DataGetFunction::UInt64Const;
   mGetFunction.ui64cFunc = giFunc;
}

template<class BoundType, class ChildType>
DataMappingFunctor<BoundType, ChildType>::DataMappingFunctor(
   SetStringFunction ssFunc, GetStringConstFunction gsFunc)
{
   // no create/add functions
   mCreateFunction = NULL;
   mAddFunction = NULL;
   
   // set set/get functions
   mSetFunction.type = DataSetFunction::String;
   mSetFunction.sFunc = ssFunc;
   mGetFunction.type = DataGetFunction::StringConst;
   mGetFunction.scFunc = gsFunc;
}

template<class BoundType, class ChildType>
DataMappingFunctor<BoundType, ChildType>::~DataMappingFunctor()
{
}

template<class BoundType, class ChildType>
void* DataMappingFunctor<BoundType, ChildType>::createChild(void* bObject)
{
   void* rval = NULL;
   
   if(mCreateFunction != NULL)
   {
      rval = (((BoundType*)bObject)->*mCreateFunction)();
   }
   
   return rval;
}

template<class BoundType, class ChildType>
void DataMappingFunctor<BoundType, ChildType>::addChild(
   void* bObject, void* cObject)
{
   if(mAddFunction != NULL)
   {
      (((BoundType*)bObject)->*mAddFunction)((ChildType*)cObject);
   }
}

template<class BoundType, class ChildType>
void DataMappingFunctor<BoundType, ChildType>::setData(
   void* bObject, const char* data, int length)
{
   BoundType* bObj = (BoundType*)bObject;
   
   // add null-terminator to data
   char d[length + 1];
   strncpy(d, data, length);
   memset(d + length, 0, 1);
   
   switch(mSetFunction.type)
   {
      case DataSetFunction::None:
         // no set function
         break;
      case DataSetFunction::Boolean:
         // convert data to boolean
         if(strcasecmp(d, "true") == 0)
         {
            (bObj->*mSetFunction.bFunc)(true);
         }
         else if(strcasecmp(d, "false") == 0)
         {
            (bObj->*mSetFunction.bFunc)(false);
         }
         else
         {
            (bObj->*mSetFunction.bFunc)(strcasecmp(d, "1") == 0);
         }
         break;
      case DataSetFunction::Int32:
         // convert data to integer
         (bObj->*mSetFunction.i32Func)(strtol(d, NULL, 10));
         break;
      case DataSetFunction::UInt32:
         // convert data to integer
         (bObj->*mSetFunction.ui32Func)(strtoul(d, NULL, 10));
         break;
      case DataSetFunction::Int64:
         // convert data to integer
         (bObj->*mSetFunction.i64Func)(strtoll(d, NULL, 10));
         break;
      case DataSetFunction::UInt64:
         // convert data to integer
         (bObj->*mSetFunction.ui64Func)(strtoull(d, NULL, 10));
         break;
      case DataSetFunction::String:
         (bObj->*mSetFunction.sFunc)(d);
         break;
   }
}

template<class BoundType, class ChildType>
void DataMappingFunctor<BoundType, ChildType>::setRawData(
   void* bObject, char* data, int length)
{
   BoundType* bObj = (BoundType*)bObject;
   
   // add null-terminator to data
   char d[length + 1];
   strncpy(d, data, length);
   memset(d + length, 0, 1);
   
   switch(mSetFunction.type)
   {
      case DataSetFunction::None:
         // no set function
         break;
      case DataSetFunction::Boolean:
         (bObj->*mSetFunction.bFunc)(*((bool*)data));
         break;
      case DataSetFunction::Int32:
         (bObj->*mSetFunction.i32Func)(*((int*)data));
         break;
      case DataSetFunction::UInt32:
         (bObj->*mSetFunction.ui32Func)(*((unsigned int*)data));
         break;
      case DataSetFunction::Int64:
         (bObj->*mSetFunction.i64Func)(*((long long*)data));
         break;
      case DataSetFunction::UInt64:
         // convert data to integer
         (bObj->*mSetFunction.ui64Func)(*((unsigned long long*)data));
         break;
      case DataSetFunction::String:
         (bObj->*mSetFunction.sFunc)(d);
         break;
   }
}

template<class BoundType, class ChildType>
void DataMappingFunctor<BoundType, ChildType>::appendData(
   void* bObject, const char* data, int length)
{
   // append data to cache
   mDataCache.put(data, length, true);
}

template<class BoundType, class ChildType>
void DataMappingFunctor<BoundType, ChildType>::endData(void* bObject)
{
   // set data
   setData(bObject, mDataCache.data(), mDataCache.length());
   
   // free data cache
   mDataCache.free();
}

template<class BoundType, class ChildType>
void DataMappingFunctor<BoundType, ChildType>::getData(void* bObject, char** s)
{
   *s = NULL;
   const char* str = NULL;
   BoundType* bObj = (BoundType*)bObject;
   
   switch(mGetFunction.type)
   {
      case DataGetFunction::None:
         // no get function
         *s = new char[1];
         memset(*s, 0, 1);
         break;
      case DataGetFunction::Boolean:
         // convert boolean to string
         *s = ((bObj->*mGetFunction.bFunc)()) ? 
            strdup("true") : strdup("false");
         break;
      case DataGetFunction::Int32:
         // convert integer to string
         *s = new char[22];
         sprintf(*s, "%d", (bObj->*mGetFunction.i32Func)());
         break;
      case DataGetFunction::UInt32:
         // convert integer to string
         *s = new char[22];
         sprintf(*s, "%du", (bObj->*mGetFunction.ui32Func)());
         break;
      case DataGetFunction::Int64:
         // convert integer to string
         *s = new char[22];
         sprintf(*s, "%lld", (bObj->*mGetFunction.i64Func)());
         break;
      case DataGetFunction::UInt64:
         // convert integer to string
         *s = new char[22];
         sprintf(*s, "%llu", (bObj->*mGetFunction.ui64Func)());
         break;
      case DataGetFunction::String:
         str = (bObj->*mGetFunction.sFunc)();
         *s = (str != NULL) ? strdup(str) : strdup("");
         break;
      case DataGetFunction::BooleanConst:
         // convert boolean to string
         *s = ((bObj->*mGetFunction.bcFunc)()) ?
            strdup("true") : strdup("false");
         break;
      case DataGetFunction::Int32Const:
         // convert integer to string
         *s = new char[22];
         sprintf(*s, "%d", (bObj->*mGetFunction.i32cFunc)());
         break;
      case DataGetFunction::UInt32Const:
         // convert integer to string
         *s = new char[22];
         sprintf(*s, "%du", (bObj->*mGetFunction.ui32cFunc)());
         break;
      case DataGetFunction::Int64Const:
         // convert integer to string
         *s = new char[22];
         sprintf(*s, "%lld", (bObj->*mGetFunction.i64cFunc)());
         break;
      case DataGetFunction::UInt64Const:
         // convert integer to string
         *s = new char[22];
         sprintf(*s, "%llu", (bObj->*mGetFunction.ui64cFunc)());
         break;
      case DataGetFunction::StringConst:
         str = (bObj->*mGetFunction.scFunc)();
         *s = (str != NULL) ? strdup(str) : strdup("");
         break;
   }
}

template<class BoundType, class ChildType>
void DataMappingFunctor<BoundType, ChildType>::getRawData(
   void* bObject, char** s)
{
   *s = NULL;
   const char* str = NULL;
   BoundType* bObj = (BoundType*)bObject;
   
   switch(mGetFunction.type)
   {
      case DataGetFunction::None:
         // no get function
         break;
      case DataGetFunction::Boolean:
         *s = (char*)new bool((bObj->*mGetFunction.bFunc)());
         break;
      case DataGetFunction::Int32:
         *s = (char*)new int((bObj->*mGetFunction.i32Func)());
         break;
      case DataGetFunction::UInt32:
         *s = (char*)new unsigned int((bObj->*mGetFunction.ui32Func)());
         break;
      case DataGetFunction::Int64:
         *s = (char*)new long long((bObj->*mGetFunction.i64Func)());
         break;
      case DataGetFunction::UInt64:
         *s = (char*)new unsigned long long((bObj->*mGetFunction.i64Func)());
         break;
      case DataGetFunction::String:
         str = (bObj->*mGetFunction.sFunc)();
         *s = (str != NULL) ? strdup(str) : strdup("");
         break;
      case DataGetFunction::BooleanConst:
         *s = (char*)new bool((bObj->*mGetFunction.bcFunc)());
         break;
      case DataGetFunction::Int32Const:
         *s = (char*)new int((bObj->*mGetFunction.i32cFunc)());
         break;
      case DataGetFunction::UInt32Const:
         *s = (char*)new unsigned int((bObj->*mGetFunction.ui32cFunc)());
         break;
      case DataGetFunction::Int64Const:
         *s = (char*)new long long((bObj->*mGetFunction.i64cFunc)());
         break;
      case DataGetFunction::UInt64Const:
         *s = (char*)new unsigned long long((bObj->*mGetFunction.i64cFunc)());
         break;
      case DataGetFunction::StringConst:
         str = (bObj->*mGetFunction.scFunc)();
         *s = (str != NULL) ? strdup(str) : strdup("");
         break;
   }
}

template<class BoundType, class ChildType>
bool DataMappingFunctor<BoundType, ChildType>::writeData(
   void* bObject, db::io::OutputStream* os)
{
   bool rval = false;
   
   const char* str = NULL;
   BoundType* bObj = (BoundType*)bObject;
   
   char s[22];
   switch(mGetFunction.type)
   {
      case DataGetFunction::None:
         // no data to write
         rval = true;
         break;
      case DataGetFunction::Boolean:
         // convert boolean to string
         if((bObj->*mGetFunction.bFunc)())
         {
            // write "true" to output stream
            rval = os->write("true", 4);
         }
         else
         {
            // write "false" to output stream
            rval = os->write("false", 4);
         }
         break;
      case DataGetFunction::Int32:
         // convert integer to string and write to output stream
         sprintf(s, "%d", (bObj->*mGetFunction.i32Func)());
         rval = os->write(s, strlen(s));
         break;
      case DataGetFunction::UInt32:
         // convert integer to string and write to output stream
         sprintf(s, "%du", (bObj->*mGetFunction.ui32Func)());
         rval = os->write(s, strlen(s));
         break;
      case DataGetFunction::Int64:
         // convert integer to string and write to output stream
         sprintf(s, "%lld", (bObj->*mGetFunction.i64Func)());
         rval = os->write(s, strlen(s));
         break;
      case DataGetFunction::UInt64:
         // convert integer to string and write to output stream
         sprintf(s, "%llu", (bObj->*mGetFunction.ui64Func)());
         rval = os->write(s, strlen(s));
         break;
      case DataGetFunction::String:
         str = (bObj->*mGetFunction.sFunc)();
         if(str != NULL)
         {
            // write string to output stream
            rval = os->write(str, strlen(str));
         }
         else
         {
            // no data to write
            rval = true;
         }
         break;
      case DataGetFunction::BooleanConst:
         // convert boolean to string
         if((bObj->*mGetFunction.bcFunc)())
         {
            // write "true" to output stream
            rval = os->write("true", 4);
         }
         else
         {
            // write "false" to output stream
            rval = os->write("false", 4);
         }
         break;
      case DataGetFunction::Int32Const:
         // convert integer to string and write to output stream
         sprintf(s, "%d", (bObj->*mGetFunction.i32cFunc)());
         rval = os->write(s, strlen(s));
         break;
      case DataGetFunction::UInt32Const:
         // convert integer to string and write to output stream
         sprintf(s, "%du", (bObj->*mGetFunction.ui32cFunc)());
         rval = os->write(s, strlen(s));
         break;
      case DataGetFunction::Int64Const:
         // convert integer to string and write to output stream
         sprintf(s, "%lld", (bObj->*mGetFunction.i64cFunc)());
         rval = os->write(s, strlen(s));
         break;
      case DataGetFunction::UInt64Const:
         // convert integer to string and write to output stream
         sprintf(s, "%llu", (bObj->*mGetFunction.ui64cFunc)());
         rval = os->write(s, strlen(s));
         break;
      case DataGetFunction::StringConst:
         str = (bObj->*mGetFunction.scFunc)();
         if(str != NULL)
         {
            // write string to output stream
            rval = os->write(str, strlen(str));
         }
         else
         {
            // no data to write
            rval = true;
         }
         break;
   }
   
   return rval;
}

template<class BoundType, class ChildType>
bool DataMappingFunctor<BoundType, ChildType>::hasData(void* bObject)
{
   bool rval = false;
   
   const char* str = NULL;
   BoundType* bObj = (BoundType*)bObject;
   
   switch(mGetFunction.type)
   {
      case DataGetFunction::None:
         // no data
         break;
      case DataGetFunction::Boolean:
      case DataGetFunction::Int32:
      case DataGetFunction::UInt32:
      case DataGetFunction::Int64:
      case DataGetFunction::UInt64:
      case DataGetFunction::BooleanConst:
      case DataGetFunction::Int32Const:
      case DataGetFunction::UInt32Const:
      case DataGetFunction::Int64Const:
      case DataGetFunction::UInt64Const:
         // has data
         rval = true;
         break;
      case DataGetFunction::String:
         str = (bObj->*mGetFunction.sFunc)();
         rval = (str != NULL && strlen(str) > 0);
         break;
      case DataGetFunction::StringConst:
         str = (bObj->*mGetFunction.scFunc)();
         rval = (str != NULL && strlen(str) > 0);
         break;
   }
   
   return rval;
}

template<class BoundType, class ChildType>
DataMapping::DataType DataMappingFunctor<BoundType, ChildType>::getDataType()
{
   DataType rval;
   
   switch(mGetFunction.type)
   {
      case DataGetFunction::None:
      case DataGetFunction::String:
      case DataGetFunction::StringConst:
         rval = String;
         break;
      case DataGetFunction::Boolean:
      case DataGetFunction::BooleanConst:
         rval = Boolean;
         break;
      case DataGetFunction::Int32:
      case DataGetFunction::Int32Const:
         rval = Int32;
         break;
      case DataGetFunction::UInt32:
      case DataGetFunction::UInt32Const:
         rval = UInt32;
         break;
      case DataGetFunction::Int64:
      case DataGetFunction::Int64Const:
         rval = Int64;
         break;
      case DataGetFunction::UInt64:
      case DataGetFunction::UInt64Const:
         rval = UInt64;
         break;
   }
   
   return rval;
}

template<class BoundType, class ChildType>
bool DataMappingFunctor<BoundType, ChildType>::isChildMapping()
{
   return mCreateFunction != NULL;
}

} // end namespace data
} // end namespace db
#endif
