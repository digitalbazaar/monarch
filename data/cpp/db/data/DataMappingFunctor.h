/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_data_DataMappingFunctor_H
#define db_data_DataMappingFunctor_H

#include "db/data/DataMapping.h"

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
   typedef void (BoundType::*SetIntegerFunction)(int);
   typedef void (BoundType::*SetStringFunction)(const char*);
   
   /**
    * Typedefs for getting data in an object.
    */
   typedef bool (BoundType::*GetBooleanFunction)();
   typedef int (BoundType::*GetIntegerFunction)();
   typedef const char* (BoundType::*GetStringFunction)();
   
   /**
    * A function for setting data in an object.
    */
   typedef struct DataSetFunction
   {
      /**
       * The type of data the function sets.
       */
      enum Type {None, Boolean, Integer, String} type;
      
      /**
       * The pointer to the set function.
       */
      union
      {
         SetBooleanFunction bFunc;
         SetIntegerFunction iFunc;
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
      enum Type {None, Boolean, Integer, String} type;
      
      /**
       * The pointer to the get function.
       */
      union
      {
         GetBooleanFunction bFunc;
         GetIntegerFunction iFunc;
         GetStringFunction sFunc;
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
      SetIntegerFunction siFunc, GetIntegerFunction giFunc);
   
   /**
    * Creates a new DataMappingFunctor.
    * 
    * @param ssFunc a function for setting data in an object.
    * @param gsFunc a function for getting data from an object.
    */
   DataMappingFunctor(
      SetStringFunction ssFunc, GetStringFunction gsFunc);
   
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
    * Appends the passed data to the bound object.
    * 
    * @param bObject the bound object.
    * @param data the data to set in the object.
    * @param length the length of the data.
    */
   virtual void appendData(void* bObject, const char* data, int length);
   
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
   SetIntegerFunction siFunc, GetIntegerFunction giFunc)
{
   // no create/add functions
   mCreateFunction = NULL;
   mAddFunction = NULL;
   
   // set set/get functions
   mSetFunction.type = DataSetFunction::Integer;
   mSetFunction.iFunc = siFunc;
   mGetFunction.type = DataGetFunction::Integer;
   mGetFunction.iFunc = giFunc;
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
      case DataSetFunction::Integer:
         // convert data to integer
         (bObj->*mSetFunction.iFunc)(strtol(d, NULL, 10));
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
   // get existing data
   char* oldData;
   getData(bObject, &oldData);
   int oldLength = strlen(oldData);
   
   char* d = NULL;
   if(oldLength > 0)
   {
      // append new data and null-terminator
      d = new char[oldLength + length + 1];
      strncpy(d, oldData, oldLength);
      strncpy(d + oldLength, data, length);
      memset(d + oldLength + length, 0, 1);
      length += oldLength;
   }
   else
   {
      // append null terminator
      d = new char[length + 1];
      strncpy(d, data, length);
      memset(d + length, 0, 1);
   }
   
   // FIXME: setData() cannot be called from here due to some strange
   // linking error where its address is returned as NULL at runtime
   BoundType* bObj = (BoundType*)bObject;
   
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
      case DataSetFunction::Integer:
         // convert data to integer
         (bObj->*mSetFunction.iFunc)(strtol(d, NULL, 10));
         break;
      case DataSetFunction::String:
         (bObj->*mSetFunction.sFunc)(d);
         break;
   }
   
   // clean up temp and old data
   delete [] d;
   delete [] oldData;
}

template<class BoundType, class ChildType>
void DataMappingFunctor<BoundType, ChildType>::getData(void* bObject, char** s)
{
   *s = NULL;
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
         if((bObj->*mGetFunction.bFunc)())
         {
            *s = new char[5];
            strcpy(*s, "true");
         }
         else
         {
            *s = new char[6];
            strcpy(*s, "false");
         }
         break;
      case DataGetFunction::Integer:
         // convert integer to string
         *s = new char[20];
         sprintf(*s, "%d", (bObj->*mGetFunction.iFunc)());
         break;
      case DataGetFunction::String:
         const char* str = (bObj->*mGetFunction.sFunc)();
         if(str != NULL)
         {
            *s = new char[strlen(str) + 1];
            strcpy(*s, str);
         }
         else
         {
            *s = new char[1];
            memset(*s, 0, 1);
         }
         break;
   }
}

template<class BoundType, class ChildType>
bool DataMappingFunctor<BoundType, ChildType>::writeData(
   void* bObject, db::io::OutputStream* os)
{
   bool rval = false;
   
   BoundType* bObj = (BoundType*)bObject;
   
   char s[20];
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
      case DataGetFunction::Integer:
         // convert integer to string and write to output stream
         sprintf(s, "%d", (bObj->*mGetFunction.iFunc)());
         rval = os->write(s, strlen(s));
         break;
      case DataGetFunction::String:
         const char* str = (bObj->*mGetFunction.sFunc)();
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
   
   BoundType* bObj = (BoundType*)bObject;
   
   switch(mGetFunction.type)
   {
      case DataGetFunction::None:
         // no data
         break;
      case DataGetFunction::Boolean:
         // has data
         rval = true;
         break;
      case DataGetFunction::Integer:
         // has data
         rval = true;
         break;
      case DataGetFunction::String:
         const char* str = (bObj->*mGetFunction.sFunc)();
         rval = (str != NULL && strlen(str) > 0);
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
