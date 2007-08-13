/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_data_DataMappingFunctor_H
#define db_data_DataMappingFunctor_H

#include "DataMapping.h"

namespace db
{
namespace data
{

/**
 * A DataMappingFunctor allows a specific object to have its data binding
 * methods associated with a universal name (namespace + name).
 * 
 * @author Dave Longley
 */
template<class T> class DataMappingFunctor : public DataMapping
{
protected:
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
         void (T::*bFunc)(bool);
         void (T::*iFunc)(int);
         void (T::*sFunc)(const char*);
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
         bool (T::*bFunc)(void);
         int (T::*iFunc)(void);
         const char* (T::*sFunc)(void);
      };
   };
   
   /**
    * The object associated with this mapping.
    */
   T* mObject;
   
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
    * Creates a new DataMappingFunctor for the given object.
    * 
    * @param obj a pointer to the object.
    */
   DataMappingFunctor(T* obj);
   
   /**
    * Destructs this DataMappingFunctor.
    */
   virtual ~DataMappingFunctor();
   
   /**
    * Sets the data set function for this functor to set a boolean.
    * 
    * @param func the set function to use.
    */
   virtual void setBooleanSetFunction(void (T::*func)(bool));
   
   /**
    * Sets the data get function for this functor to get a boolean.
    * 
    * @param func the get function to use.
    */
   virtual void setBooleanGetFunction(bool (T::*func)(void));
   
   /**
    * Sets the data set function for this functor to set an integer.
    * 
    * @param func the set function to use.
    */
   virtual void setIntegerSetFunction(void (T::*func)(int));
   
   /**
    * Sets the data get function for this functor to get an integer.
    * 
    * @param func the get function to use.
    */
   virtual void setIntegerGetFunction(int (T::*func)(void));
   
   /**
    * Sets the data set function for this functor to set a string.
    * 
    * @param func the set function to use.
    */
   virtual void setStringSetFunction(void (T::*func)(const char*));
   
   /**
    * Sets the data get function for this functor to get a string.
    * 
    * @param func the get function to use.
    */
   virtual void setStringGetFunction(const char* (T::*func)(void));
   
   /**
    * Clears some objects from a collection within the bound object.
    */
   virtual void clearCollection();
   
   /**
    * Updates a collection of the bound object.
    */
   virtual void updateCollection();
   
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

template<class T>
DataMappingFunctor<T>::DataMappingFunctor(T* obj)
{
   // store object
   mObject = obj;
   
   // no set or get functions yet
   mSetFunction.type = DataSetFunction::None;
   mGetFunction.type = DataGetFunction::None;
}

template<class T>
DataMappingFunctor<T>::~DataMappingFunctor()
{
}

template<class T>
void DataMappingFunctor<T>::setBooleanSetFunction(void (T::*func)(bool))
{
   mSetFunction.type = DataSetFunction::Boolean;
   mSetFunction.bFunc = func;
}

template<class T>
void DataMappingFunctor<T>::setBooleanGetFunction(bool (T::*func)(void))
{
   mGetFunction.type = DataGetFunction::Boolean;
   mGetFunction.bFunc = func;
}

template<class T>
void DataMappingFunctor<T>::setIntegerSetFunction(void (T::*func)(int))
{
   mSetFunction.type = DataSetFunction::Integer;
   mSetFunction.iFunc = func;
}

template<class T>
void DataMappingFunctor<T>::setIntegerGetFunction(int (T::*func)(void))
{
   mGetFunction.type = DataGetFunction::Integer;
   mGetFunction.iFunc = func;
}

template<class T>
void DataMappingFunctor<T>::setStringSetFunction(void (T::*func)(const char*))
{
   mSetFunction.type = DataSetFunction::String;
   mSetFunction.sFunc = func;
}

template<class T>
void DataMappingFunctor<T>::setStringGetFunction(const char* (T::*func)(void))
{
   mGetFunction.type = DataGetFunction::String;
   mGetFunction.sFunc = func;
}

template<class T>
void DataMappingFunctor<T>::clearCollection()
{
//   if(mClearCollectionFunction != NULL)
//   {
//      (mObject->*mClearCollectionFunction)();
//   }
}

template<class T>
void DataMappingFunctor<T>::updateCollection()
{
//   if(mUpdateCollectionFunction != NULL)
//   {
//      (mObject->*mUpdateCollectionFunction)();
//   }
}

template<class T>
void DataMappingFunctor<T>::setData(const char* data)
{
   switch(mSetFunction.type)
   {
      case DataSetFunction::None:
         // no set function
         break;
      case DataSetFunction::Boolean:
         // FIXME: convert data to boolean
         (mObject->*mSetFunction.bFunc)(data);
         break;
      case DataSetFunction::Integer:
         // FIXME: convert data to integer
         //(mObject->*mSetFunction.iFunc)(data);
         break;
      case DataSetFunction::String:
         //(mObject->*mSetFunction.sFunc)(data);
         break;
   }
}

template<class T>
const char* DataMappingFunctor<T>::getData()
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
