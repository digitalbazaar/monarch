/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_data_DataMapping_H
#define db_data_DataMapping_H

#include "db/io/OutputStream.h"

#include <list>

namespace db
{
namespace data
{

/**
 * A DataMapping maps a universal name (namespace + name) to functions
 * for binding the data to an object.
 * 
 * @author Dave Longley
 */
class DataMapping
{
public:
   /**
    * Possible data types.
    */
   typedef enum DataType
   {
      Boolean, Int32, UInt32, Int64, UInt64, String
   };
   
   /**
    * Creates a new DataMapping.
    */
   DataMapping() {};
   
   /**
    * Destructs this DataMapping.
    */
   virtual ~DataMapping() {};
   
   /**
    * Creates a child object to add to the bound object.
    * 
    * @param bObject the bound object.
    * 
    * @return a pointer to the child object.
    */
   virtual void* createChild(void* bObject) = 0;
   
   /**
    * Adds a child object to the bound object.
    * 
    * @param bObject the bound object.
    * @param cObject the child object to add to the bound object.
    */
   virtual void addChild(void* bObject, void* cObject) = 0;
   
   /**
    * Sets the passed data in the bound object.
    * 
    * @param bObject the bound object.
    * @param data the data to set in the object.
    * @param length the length of the data.
    */
   virtual void setData(void* bObject, const char* data, int length) = 0;
   
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
   virtual void setRawData(void* bObject, char* data, int length) = 0;
   
   /**
    * Appends the passed data to the bound object.
    * 
    * @param bObject the bound object.
    * @param data the data to set in the object.
    * @param length the length of the data.
    */
   virtual void appendData(void* bObject, const char* data, int length) = 0;
   
   /**
    * Ends data being set to the bound object. This method should flush any
    * cached data (via appendData()) to the bound object.
    * 
    * @param bObject the bound object.
    */
   virtual void endData(void* bObject) = 0;
   
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
   virtual void getData(void* bObject, char** s) = 0;
   
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
   virtual void getRawData(void* bObject, char** s) = 0;
   
   /**
    * Writes the data for the passed bound object to the given output stream.
    * 
    * @param bObject the bound object with data to write out.
    * @param os the OutputStream to write to.
    */
   virtual bool writeData(void* bObject, db::io::OutputStream* os) = 0;
   
   /**
    * Returns true if the passed bound object has data, false if not.
    * 
    * @param bObject the bound object to check for data.
    * 
    * @return true if the passed bound object has data, false if not.
    */
   virtual bool hasData(void* bObject) = 0;
   
   /**
    * Returns the type of data for this mapping.
    * 
    * @return the type of data for this mapping.
    */
   virtual DataType getDataType() = 0;
   
   /**
    * True if this DataMapping is a create/add child mapping, false if it is a
    * set/get mapping. 
    * 
    * @return true if this DataMapping is a create/add child mapping, false if
    *         it is a set/get mapping.
    */
   virtual bool isChildMapping() = 0;
};

} // end namespace data
} // end namespace db
#endif
