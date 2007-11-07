/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_data_DynamicObjectBinding_H
#define db_data_DynamicObjectBinding_H

#include "db/data/DataBinding.h"
#include "db/data/DataMappingFunctor.h"
#include "db/util/DynamicObject.h"

namespace db
{
namespace data
{

/**
 * A DynamicObjectBinding provides the data bindings for a DynamicObject so
 * that it can be serialized and/or deserialized.
 * 
 * @author Dave Longley
 */
class DynamicObjectBinding : public db::data::DataBinding
{
protected:
   /**
    * The DataMapping for this binding.
    */
   db::data::DataMappingFunctor<DynamicObjectBinding> mNameMapping;
   db::data::DataMappingFunctor<DynamicObjectBinding, db::util::DynamicObject>
      mCreateMapping;
   db::data::DataMappingFunctor<DynamicObjectBinding> mStringMapping;
   db::data::DataMappingFunctor<DynamicObjectBinding> mNumberMapping;
   db::data::DataMappingFunctor<DynamicObjectBinding> mBooleanMapping;
   
   /**
    * The stack of names read in.
    */
   std::list<std::string> mNameStack;
   
public:
   /**
    * Creates a new DynamicObjectBinding.
    * 
    * @param dyno the DynamicObject this binding is for.
    */
   DynamicObjectBinding(db::util::DynamicObject& dyno);
   
   /**
    * Destructs this DynamicObjectBinding.
    */
   virtual ~DynamicObjectBinding();
   
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
   virtual void* getCreateAddObject(db::data::DataName* dn);
   
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
    * Creates a DynamicObject.
    * 
    * @return the heap-allocated DynamicObject.
    */
   db::util::DynamicObject* createDynamicObject();
   
   /**
    * Adds a DynamicObject to the underlying DynamicObject.
    * 
    * @param dyno the DynamicObject to add.
    */
   virtual void addDynamicObject(db::util::DynamicObject* dyno);
   
   /**
    * Sets the name for the DynamicObject.
    * 
    * @param name the name to set.
    */
   virtual void setName(const char* name);
   
   /**
    * Gets the name for the DynamicObject.
    * 
    * @return the name.
    */
   virtual const char* getName();
   
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
    * Populates a list of child objects for the given DataName. This method
    * must be overloaded to populate child objects with data using this binding.
    *
    * @param dn the DataName to get the child objects for.
    * @param children the list to store child objects in.
    */
   virtual void getChildren(db::data::DataName* dn, std::list<void*>& children);
};

} // end namespace data
} // end namespace db
#endif
