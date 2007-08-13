/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_data_DataMappingFunctor_H
#define db_data_DataMappingFunctor_H

#include "DataMapping.h"

// FIXME: remove me
#include <iostream>

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
template<class T, class TObject = void>
class DataMappingFunctor : public DataMapping
{
protected:
   /**
    * Typedef for creating another object.
    */
   typedef TObject* (T::*CreateObjectFunction)(void);
   
   /**
    * Typedef for adding another object.
    */
   typedef void (T::*AddObjectFunction)(TObject*);
   
   /**
    * Typedefs for setting data in an object.
    */
   typedef void (T::*SetBooleanFunction)(bool); 
   typedef void (T::*SetIntegerFunction)(int);
   typedef void (T::*SetStringFunction)(const char*);
   
   /**
    * Typedefs for getting data in an object.
    */
   typedef bool (T::*GetBooleanFunction)(void);
   typedef int (T::*GetIntegerFunction)(void);
   typedef const char* (T::*GetStringFunction)(void);
   
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
    * The object associated with this mapping.
    */
   T* mObject;
   
   /**
    * A function for creating another object.
    */
   CreateObjectFunction mCreateFunction;
   
   /**
    * A function for adding another object.
    */
   AddObjectFunction mAddFunction;
   
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
    * @param cFunc a function for creating another object.
    * @param aFunc a function for adding another object to the bound object.
    */
   DataMappingFunctor(
      CreateObjectFunction cFunc, AddObjectFunction aFunc);
   
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
    * Sets the bound object.
    * 
    * @param obj the bound object to use with this mapping.
    */
   virtual void setObject(void* obj);
   
   /**
    * Creates an object to add to the bound object.
    * 
    * @return a pointer to the object.
    */
   virtual void* createObject();
   
   /**
    * Adds an object to the bound object.
    * 
    * @param obj the object to add to the bound object.
    */
   virtual void addObject(void* obj);
   
   /**
    * Sets the passed data in the bound object.
    * 
    * @param data the data to set in the object.
    */
   virtual void setData(const char* data);
   
   /**
    * Gets data from the bound object.
    * 
    * @return the data from the bound object.
    */
   virtual const char* getData();
};

template<class T, class TObject>
DataMappingFunctor<T, TObject>::DataMappingFunctor(
   CreateObjectFunction cFunc, AddObjectFunction aFunc)
{
   // set create/add functions
   mCreateFunction = cFunc;
   mAddFunction = aFunc;
   
   // no set/get functions
   mSetFunction.type = DataSetFunction::None;
   mGetFunction.type = DataGetFunction::None;
}

template<class T, class TObject>
DataMappingFunctor<T, TObject>::DataMappingFunctor(
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

template<class T, class TObject>
DataMappingFunctor<T, TObject>::DataMappingFunctor(
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

template<class T, class TObject>
DataMappingFunctor<T, TObject>::DataMappingFunctor(
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

template<class T, class TObject>
DataMappingFunctor<T, TObject>::~DataMappingFunctor()
{
}

template<class T, class TObject>
void DataMappingFunctor<T, TObject>::setObject(void* obj)
{
   // store object
   mObject = (T*)obj;
}

template<class T, class TObject>
void* DataMappingFunctor<T, TObject>::createObject()
{
   void* rval = NULL;
   
   if(mCreateFunction != NULL)
   {
      rval = (mObject->*mCreateFunction)();
   }
   
   return rval;
}

template<class T, class TObject>
void DataMappingFunctor<T, TObject>::addObject(void* obj)
{
   if(mAddFunction != NULL)
   {
      (mObject->*mAddFunction)((TObject*)obj);
   }
}

template<class T, class TObject>
void DataMappingFunctor<T, TObject>::setData(const char* data)
{
   std::cout << "+++++ in setData()" << std::endl;
   
   switch(mSetFunction.type)
   {
      case DataSetFunction::None:
         // no set function
         break;
      case DataSetFunction::Boolean:
         // FIXME: convert data to boolean
         //(mObject->*mSetFunction.bFunc)(data);
         break;
      case DataSetFunction::Integer:
         // FIXME: convert data to integer
         //(mObject->*mSetFunction.iFunc)(data);
         break;
      case DataSetFunction::String:
         (mObject->*mSetFunction.sFunc)(data);
         break;
   }
}

template<class T, class TObject>
const char* DataMappingFunctor<T, TObject>::getData()
{
   const char* rval = NULL;
   
   switch(mGetFunction.type)
   {
      case DataGetFunction::None:
         // no get function
         break;
      case DataGetFunction::Boolean:
         (mObject->*mGetFunction.bFunc)();
         // FIXME: convert boolean to string
         break;
      case DataGetFunction::Integer:
         (mObject->*mGetFunction.iFunc)();
         // FIXME: convert integer to string
         break;
      case DataGetFunction::String:
         rval = (mObject->*mGetFunction.sFunc)();
         break;
   }
   
   return rval;
}

} // end namespace data
} // end namespace db
#endif
