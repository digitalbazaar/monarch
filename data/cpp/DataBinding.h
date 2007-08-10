/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_data_DataBinding_H
#define db_data_DataBinding_H

namespace db
{
namespace data
{

/**
 * A DataBinding provides a generic interface for converting formatted data
 * to or from an object.
 * 
 * Data can be set via this binding either in a stream-oriented fashion by
 * calling a combination of startData and appendData()s, in a one-off fashion
 * by calling setData().
 * 
 * @author Dave Longley
 */
class DataBinding
{
protected:
   /**
    * The object associated with this binding.
    */
   void* mObject;
   
   /**
    * Stores the current data namespace uri.
    */
   char* mDataNamespace;
   
   /**
    * Stores the current data name.
    */
   char* mDataName;
   
public:
   /**
    * Creates a new DataBinding for the given object.
    * 
    * @param obj a pointer to the object.
    */
   DataBinding(void* obj);
   
   /**
    * Destructs this DataBinding.
    */
   virtual ~DataBinding();
   
   /**
    * Starts setting data for the given namespace and name and provides the
    * appropriate DataBinding (which may be this DataBinding) for updating
    * the data.
    * 
    * Extending DataBindings can take whatever action is appropriate to
    * handle special a character-encoding (i.e. UTF-8), if one is provided.
    * 
    * @param charEncoding the encoding for the characters.
    * @param ns the null-terminated namespace uri for the data.
    * @param name the null-terminated name for the data.
    * 
    * @return the appropriate DataBinding for setting the data.
    */
   virtual DataBinding* startData(
      const char* charEncoding, const char* ns, const char* name);
   
   /**
    * Appends more data to the current data.
    * 
    * @param charEncoding the encoding for the data.
    * @param data the data to append.
    * @param length the length of the data.
    */
   virtual void appendData(
      const char* charEncoding, char* data, unsigned int length);
   
   /**
    * Ends setting data for the given DataBinding.
    * 
    * @param db the DataBinding that has finished having some of its data set.
    */
   virtual void endData(DataBinding* db);
   
   /**
    * Sets data with the given namespace and name.
    * 
    * @param charEncoding the encoding for the characters.
    * @param ns the null-terminated namespace for the data.
    * @param name the null-terminated name for the data.
    * @param data the data.
    * @param length the length of the data.
    */
   virtual void setData(
      const char* charEncoding, const char* ns, const char* name,
      const char* data, unsigned int length);
   
   /**
    * Gets a pointer to the object associated with this binding.
    */
   virtual void* getObject();
};

} // end namespace data
} // end namespace db
#endif
