/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_data_DataBinding_H
#define db_data_DataBinding_H

#include "DataMapping.h"

#include <map>

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
    * A DataName is a universal name for data that includes a namespace
    * and a local name.
    */
   typedef struct DataName
   {
      /**
       * The namespace.
       */
      char* ns;
      
      /**
       * The name.
       */
      char* name;
   };
   
   /**
    * A DataNameComparator compares two DataNames.
    */
   typedef struct DataNameComparator
   {
      /**
       * Compares two DataNames, returning true if the first DataName's
       * universal name is less than the second's, false if not.
       * 
       * @param dn1 the first DataName.
       * @param dn2 the second DataName.
       * 
       * @return true if the dn1 < dn2, false if not.
       */
      bool operator()(DataName* dn1, DataName* dn2) const;
   };
   
   /**
    * The bound object.
    */
   void* mObject;
   
   /**
    * A map of DataNames to DataMappings.
    */
   std::map<DataName*, DataMapping*, DataNameComparator> mDataMappings;
   
   /**
    * A map of DataNames to DataBindings.
    */
   std::map<DataName*, DataBinding*, DataNameComparator> mDataBindings;
   
   /**
    * The DataName for the current DataBinding.
    */
   DataName* mCurrentDataName;
   
   /**
    * Creates a new DataName from a namespace and a name.
    * 
    * @param ns the namespace to use.
    * @param name the name to use.
    * 
    * @return the created DataName.
    */
   DataName* createDataName(const char* ns, const char* name);
   
   /**
    * Frees a DataName.
    * 
    * @param dn the DataName to free.
    */
   void freeDataName(DataName* dn);
   
public:
   /**
    * Creates a new DataBinding for the given object.
    * 
    * @param obj the object this DataBinding is for.
    */
   DataBinding(void* obj = NULL);
   
   /**
    * Destructs this DataBinding.
    */
   virtual ~DataBinding();
   
   /**
    * Adds a data's DataMapping to this DataBinding.
    * 
    * @param ns the null-terminated namespace for the data.
    * @param name the null-terminated name for the data.
    * @param dm the DataMapping to add to this DataBinding.
    */
   virtual void addDataMapping(
      const char* ns, const char* name, DataMapping* dm);
   
   /**
    * Adds another DataBinding to this DataBinding.
    * 
    * @param ns the null-terminated namespace for the data.
    * @param name the null-terminated name for the data.
    * @param db the DataBinding to add to this DataBinding.
    */
   virtual void addDataBinding(
      const char* ns, const char* name, DataBinding* db);
   
   /**
    * Starts setting data for the given name and provides the appropriate
    * DataBinding (which may be this DataBinding) for updating the data.
    * 
    * Extending DataBindings can take whatever action is appropriate to
    * handle special a character-encoding (i.e. UTF-8), if one is provided.
    * 
    * @param charEncoding the encoding for any characters.
    * @param ns the null-terminated namespace for the data.
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
    * @param charEncoding the encoding for any characters.
    * @param ns the null-terminated namespace for the data.
    * @param name the null-terminated name for the data.
    * @param db the DataBinding that has finished having its data set.
    */
   virtual void endData(
      const char* charEncoding,
      const char* ns, const char* name, DataBinding* db);
   
   /**
    * Sets data with the given namespace and name.
    * 
    * @param charEncoding the encoding for any characters.
    * @param ns the null-terminated namespace for the data.
    * @param name the null-terminated name for the data.
    * @param data the data.
    * @param length the length of the data.
    */
   virtual void setData(
      const char* charEncoding,
      const char* ns, const char* name,
      const char* data, unsigned int length);
};

} // end namespace data
} // end namespace db
#endif
