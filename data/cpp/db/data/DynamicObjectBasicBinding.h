/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_data_DynamicObjectBasicBinding_H
#define db_data_DynamicObjectBasicBinding_H

#include "db/data/DataBinding.h"
#include "db/data/DataMappingFunctor.h"
#include "db/util/DynamicObject.h"

namespace db
{
namespace data
{

/**
 * A DynamicObjectBasicBinding is a helper binding that binds strings,
 * booleans, and numbers to a DynamicObject. 
 * 
 * @author Dave Longley
 */
class DynamicObjectBasicBinding : public db::data::DataBinding
{
protected:
   /**
    * The DataMappings for this binding.
    */
   db::data::DataMappingFunctor<DynamicObjectBasicBinding> mStringMapping;
   db::data::DataMappingFunctor<DynamicObjectBasicBinding> mBooleanMapping;
   db::data::DataMappingFunctor<DynamicObjectBasicBinding> mNumberMapping;
   
   /**
    * Stores a boolean or number converted to a string.
    */
   std::string mTempString;
   
public:
   /**
    * Creates a new DynamicObjectBasicBinding.
    * 
    * @param dyno the DynamicObject this binding is for.
    */
   DynamicObjectBasicBinding(db::util::DynamicObject* dyno = NULL);
   
   /**
    * Destructs this DynamicObjectBinding.
    */
   virtual ~DynamicObjectBasicBinding();
   
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
    * Sets a String to the DynamicObject.
    * 
    * @param str the string to set.
    */
   virtual void setString(const char* str);
   
   /**
    * Gets a String from the DynamicObject.
    * 
    * @return the string.
    */
   virtual const char* getString();
   
   /**
    * Sets a Boolean to the DynamicObject.
    * 
    * @param b the boolean to set.
    */
   virtual void setBoolean(const char* b);
   
   /**
    * Gets a Boolean to the DynamicObject.
    * 
    * @return the boolean.
    */
   virtual const char* getBoolean();
   
   /**
    * Sets a Number to the DynamicObject.
    * 
    * @param num the number to set.
    */
   virtual void setNumber(const char* num);
   
   /**
    * Gets a Number from the DynamicObject.
    * 
    * @return the number.
    */
   virtual const char* getNumber();
};

} // end namespace data
} // end namespace db
#endif
