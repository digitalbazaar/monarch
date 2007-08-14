/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_data_DataMapping_H
#define db_data_DataMapping_H

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
    * Appends the passed data to the bound object.
    * 
    * @param bObject the bound object.
    * @param data the data to set in the object.
    * @param length the length of the data.
    */
   virtual void appendData(void* bObject, const char* data, int length) = 0;
   
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
};

} // end namespace data
} // end namespace db
#endif
