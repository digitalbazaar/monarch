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
    * Clears some objects from a collection within the bound object.
    */
   virtual void clearCollection() = 0;
   
   /**
    * Updates a collection of the bound object.
    */
   virtual void updateCollection() = 0;
   
   /**
    * Sets the passed data in the bound object.
    * 
    * @param data the data to set in the object.
    */
   virtual void setData(const char* data) = 0;
   
   /**
    * Gets data from the bound object.
    * 
    * @return the data from the bound object.
    */
   virtual const char* getData() = 0;
};

} // end namespace data
} // end namespace db
#endif
