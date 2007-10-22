/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_data_DataBinding_H
#define db_data_DataBinding_H

#include "db/data/DataMapping.h"

#include <map>
#include <list>

namespace db
{
namespace data
{

/**
 * A DataName is a universal name for data that includes a namespace
 * and a local name. A flag can also be set to indicate whether the DataName
 * refers to a major or minor piece of data.
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
   
   /**
    * True if this is a major piece of data, false if it is a minor piece
    * of data.
    */
   bool major;
   
   /**
    * Returns true if this DataName is equal to the passed DataName, false
    * if not.
    * 
    * @param dn the DataName to compare to this one.
    */
   bool equals(DataName* dn);
};

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
    * The root data name, if any.
    */
   DataName* mRootDataName;
   
   /**
    * A map of DataNames to DataMappings.
    */
   typedef std::map<DataName*, DataMapping*, DataNameComparator> DataMappingMap;
   DataMappingMap mDataMappings;
   
   /**
    * A map of DataNames to DataBindings.
    */
   typedef std::map<DataName*, DataBinding*, DataNameComparator> DataBindingMap;
   DataBindingMap mDataBindings;
   
   /**
    * A list that maintains the order in which DataNames were added.
    */
   std::list<DataName*> mDataNameOrder;
   
   /**
    * The DataName for the current DataBinding.
    */
   DataName* mCurrentDataName;
   
   /**
    * Creates a new DataName from a namespace, a name, and whether or not
    * it refers to a major or minor piece of data.
    * 
    * @param ns the namespace to use.
    * @param name the name to use.
    * @param major true if the DataName refers to a major piece of data, false
    *              if it refers to a minor piece of data.
    * 
    * @return the created DataName.
    */
   virtual DataName* createDataName(
      const char* ns, const char* name, bool major);
   
   /**
    * Frees a DataName.
    * 
    * @param dn the DataName to free.
    */
   virtual void freeDataName(DataName* dn);
   
   /**
    * Returns the object to use with a create/add DataMapping. The default
    * implementation returns the bound object.
    * 
    * This can be overloaded if a special data translation is to be used
    * by a DataBinding for a given bound object. In other words, if the
    * bound object doesn't have the methods necessary to create/add a child,
    * another object (like the extended DataBinding) can provide the
    * create/add methods to update the bound object.
    * 
    * @return the object to use with a create/add DataMapping.
    */
   virtual void* getCreateAddObject(DataName* dn);
   
   /**
    * Returns the object to use with a set/get DataMapping. The default
    * implementation returns the bound object.
    * 
    * This can be overloaded if a special data translation is to be used
    * by a DataBinding for a given bound object. In other words, if the
    * bound object doesn't have the methods necessary to set/get data,
    * another object (like the extended DataBinding) can provide the
    * set/get methods to update the bound object.
    * 
    * @return the object to use with a set/get DataMapping.
    */
   virtual void* getSetGetObject(DataName* dn);
   
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
    * Adds a data's DataMapping to this DataBinding. DataMappings that have
    * associated DataBindings must be added before their associated
    * DataBindings.
    * 
    * Adding any DataMapping with the same namespace and name as one that
    * has already been added will result in a memory leak and other
    * undefined behavior.
    * 
    * @param ns the null-terminated namespace for the data.
    * @param name the null-terminated name for the data.
    * @param major true if the DataMapping is for a major piece of data,
    *              false if it is for a minor piece of data.
    * @param dm the DataMapping to add to this DataBinding.
    */
   virtual void addDataMapping(
      const char* ns, const char* name, bool major, DataMapping* dm);
   
   /**
    * Removes a data's DataMapping from this DataBinding.
    * 
    * @param ns the null-terminated namespace for the data.
    * @param name the null-terminated name for the data.
    * @param major true if the DataMapping is for a major piece of data,
    *              false if it is for a minor piece of data.
    */
   virtual void removeDataMapping(
      const char* ns, const char* name, bool major);
   
   /**
    * Adds another DataBinding to this DataBinding. DataBindings must be added
    * after their associated DataMappings.
    * 
    * Adding any DataBinding with the same namespace and name as one that
    * has already been added will result in a memory leak and other undefined
    * behavior.
    * 
    * @param ns the null-terminated namespace for the data.
    * @param name the null-terminated name for the data.
    * @param db the DataBinding to add to this DataBinding.
    */
   virtual void addDataBinding(
      const char* ns, const char* name, DataBinding* db);
   
   /**
    * Removes another DataBinding from this DataBinding.
    * 
    * @param ns the null-terminated namespace for the data.
    * @param name the null-terminated name for the data.
    */
   virtual void removeDataBinding(const char* ns, const char* name);
   
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
      const char* charEncoding, const char* data, unsigned int length);
   
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
   
   /**
    * Sets the bound object to use with this binding.
    * 
    * @param obj the bound object to use with this binding.
    */
   virtual void setObject(void* obj);
   
   /**
    * Gets the bound object used with this binding.
    * 
    * @return the bound object used with this binding.
    */
   virtual void* getObject();
   
   /**
    * Called to indicate that this binding is now being used in a
    * serialization process.
    * 
    * This callback is useful to dynamically altering mappings/bindings.
    */
   virtual void serializationStarted();
   
   /**
    * Called to indicate that this binding is now being used in a
    * deserialization process.
    * 
    * This callback is useful to dynamically altering mappings/bindings.
    */
   virtual void deserializationStarted();
   
   /**
    * Sets the root data name.
    * 
    * @param ns the null-terminated namespace for the data.
    * @param name the null-terminated name for the data.
    */
   virtual void setDataName(const char* ns, const char* name);
   
   /**
    * Gets the root DataName, if one exists.
    * 
    * @return the root DataName or NULL if none exists.
    */
   virtual DataName* getDataName();
   
   /**
    * Populates a list of child objects for the given DataName. This method
    * must be overloaded to populate child objects with data using this binding.
    * 
    * @param dn the DataName to get the child objects for.
    * @param objLchildrenist the list to store child objects in.
    */
   virtual void getChildren(DataName* dn, std::list<void*>& children);
   
   /**
    * Gets a DataMapping from the given DataName.
    * 
    * @param dn the DataName for the DataMapping to retrieve.
    * 
    * @return a DataMapping from the given DataName.
    */
   virtual DataMapping* getDataMapping(DataName* dn);
   
   /**
    * Gets a DataBinding from the given DataName.
    * 
    * @param dn the DataName for the DataBinding to retrieve.
    * 
    * @return a DataBinding from the given DataName.
    */
   virtual DataBinding* getDataBinding(DataName* dn);
   
   /**
    * Gets the DataNames for this DataBinding, in order. For DataNames that
    * have a DataMapping and a DataBinding, the DataName for the DataMapping
    * will be first in the list, followed by the DataName for the DataBinding.
    * 
    * @return the DataNames for this DataBinding, in order.
    */
   virtual std::list<DataName*>& getDataNames();
};

} // end namespace data
} // end namespace db
#endif
